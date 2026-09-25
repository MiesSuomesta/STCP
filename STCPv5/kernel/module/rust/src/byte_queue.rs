use alloc::{
    collections::VecDeque,
    vec::Vec,
};

use crate::error::StcpError;

/*
 * Wire queues used to grow a nominal 1 MiB chunk a few KiB at a time.
 * Vec::try_reserve_exact() therefore reallocated the same tail chunk on
 * virtually every W5500/TCP receive.  On Zephyr this fragmented the
 * small heap until carrier_receive() returned -ENOMEM.
 *
 * Use modest fixed-capacity chunks instead.  Capacity is allocated once
 * when a chunk is created and subsequent appends never reallocate it.
 */
pub const BYTE_QUEUE_CHUNK_SIZE: usize = 16 * 1024;

#[inline]
fn byte_queue_chunk_capacity(remaining: usize) -> usize {
    if remaining <= 256 {
        256
    } else if remaining <= 1024 {
        1024
    } else if remaining <= 4096 {
        4096
    } else {
        BYTE_QUEUE_CHUNK_SIZE
    }
}

pub struct ByteChunk {
    data: Vec<u8>,
    offset: usize,
}

pub struct ByteQueue {
    chunks: VecDeque<ByteChunk>,
    len: usize,
}

impl ByteQueue {
    pub const fn new() -> Self {
        Self {
            chunks: VecDeque::new(),
            len: 0,
        }
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.len
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    pub fn clear(&mut self) {
        self.chunks.clear();
        self.len = 0;
    }

    pub fn push_slice(
        &mut self,
        mut input: &[u8],
    ) -> Result<(), StcpError> {
        let added = input.len();
        let new_len = self.len.checked_add(added).ok_or(StcpError::NoMem)?;

        while !input.is_empty() {
            if let Some(back) = self.chunks.back_mut() {
                /*
                 * ByteQueue-created chunks reserve BYTE_QUEUE_CHUNK_SIZE once
                 * up front.  Fill only existing capacity here: never call
                 * reserve() on the hot receive path.
                 *
                 * Chunks inserted by push_vec()/push_vec_from() may have no
                 * spare capacity; those simply become immutable queue chunks
                 * and a fresh fixed-capacity tail is created below.
                 */
                let spare = back.data.capacity().saturating_sub(back.data.len());

                if spare != 0 {
                    let count = spare.min(input.len());
                    back.data.extend_from_slice(&input[..count]);
                    input = &input[count..];
                    continue;
                }
            }

            /*
             * Small control frames must not force a 16 KiB allocation.
             * Keep large stream traffic on fixed 16 KiB chunks, but size
             * small tails in bounded tiers so Handshake/ChunkEnd/Close
             * frames remain cheap even when the Zephyr heap is tight.
             */
            let capacity = byte_queue_chunk_capacity(input.len());
            let count = input.len().min(capacity);
            let mut chunk = Vec::new();
            chunk
                .try_reserve_exact(capacity)
                .map_err(|_| StcpError::NoMem)?;
            chunk.extend_from_slice(&input[..count]);
            self.chunks.push_back(ByteChunk {
                data: chunk,
                offset: 0,
            });
            input = &input[count..];
        }

        self.len = new_len;
        Ok(())
    }

    pub fn extend<I>(&mut self, input: I)
    where
        I: IntoIterator<Item = u8>,
    {
        let mut chunk = Vec::new();

        for byte in input {
            if chunk.len() == BYTE_QUEUE_CHUNK_SIZE {
                let count = chunk.len();

                self.chunks.push_back(ByteChunk {
                    data: chunk,
                    offset: 0,
                });

                self.len = self.len.saturating_add(count);
                chunk = Vec::new();
            }

            chunk.push(byte);
        }

        if !chunk.is_empty() {
            let count = chunk.len();

            self.chunks.push_back(ByteChunk {
                data: chunk,
                offset: 0,
            });

            self.len = self.len.saturating_add(count);
        }
    }


    /// Append an owned buffer without copying its contents.
    pub fn push_vec(&mut self, input: Vec<u8>) -> Result<(), StcpError> {
        if input.is_empty() {
            return Ok(());
        }

        let count = input.len();
        self.len = self.len.checked_add(count).ok_or(StcpError::NoMem)?;
        self.chunks.push_back(ByteChunk {
            data: input,
            offset: 0,
        });
        Ok(())
    }

    /// Append a range from an owned buffer without copying. Bytes before
    /// `offset` remain allocated but are skipped by the queue reader.
    pub fn push_vec_from(
        &mut self,
        input: Vec<u8>,
        offset: usize,
    ) -> Result<(), StcpError> {
        if offset > input.len() {
            return Err(StcpError::Protocol);
        }
        let count = input.len() - offset;
        if count == 0 {
            return Ok(());
        }
        self.len = self.len.checked_add(count).ok_or(StcpError::NoMem)?;
        self.chunks.push_back(ByteChunk { data: input, offset });
        Ok(())
    }

    /// Remove up to `count` bytes using chunk-sized advances.
    pub fn discard(&mut self, mut count: usize) -> usize {
        let requested = count;

        while count != 0 {
            let Some(front) = self.chunks.front_mut() else {
                break;
            };

            let available = front.data.len() - front.offset;
            let consumed = available.min(count);
            front.offset += consumed;
            count -= consumed;
            self.len -= consumed;

            if front.offset == front.data.len() {
                self.chunks.pop_front();
            }
        }

        requested - count
    }

    /// Detach the first owned chunk without copying when it exactly matches
    /// `count`. This is the stream fast path for a complete frame payload.
    pub fn take_exact_front_vec(&mut self, count: usize) -> Option<Vec<u8>> {
        let front = self.chunks.front()?;
        if front.offset != 0 || front.data.len() != count {
            return None;
        }

        let chunk = self.chunks.pop_front()?;
        self.len = self.len.saturating_sub(count);
        Some(chunk.data)
    }

    /// Return exactly `count` bytes, detaching an exact front chunk when
    /// possible and falling back to a copy for fragmented stream input.
    pub fn take_or_read_vec(&mut self, count: usize) -> Result<Vec<u8>, StcpError> {
        if let Some(buffer) = self.take_exact_front_vec(count) {
            return Ok(buffer);
        }
        self.read_vec(count)
    }

    /// Copy exactly `count` bytes into one owned buffer.
    pub fn read_vec(&mut self, count: usize) -> Result<Vec<u8>, StcpError> {
        if self.len < count {
            return Err(StcpError::Again);
        }

        let mut output = Vec::new();
        output.try_reserve_exact(count).map_err(|_| StcpError::NoMem)?;
        output.resize(count, 0);

        if self.read_into(&mut output) != count {
            return Err(StcpError::Protocol);
        }

        Ok(output)
    }

    pub fn pop_front(&mut self) -> Option<u8> {
        let front = self.chunks.front_mut()?;
        let value = front.data.get(front.offset).copied()?;

        front.offset += 1;
        self.len = self.len.saturating_sub(1);

        if front.offset == front.data.len() {
            self.chunks.pop_front();
        }

        Some(value)
    }

    pub fn read_into(&mut self, output: &mut [u8]) -> usize {
        let mut written = 0;

        while written < output.len() {
            let Some(front) = self.chunks.front_mut() else {
                break;
            };

            let available = front.data.len() - front.offset;
            let count = available.min(output.len() - written);

            output[written..written + count].copy_from_slice(
                &front.data[front.offset..front.offset + count],
            );

            front.offset += count;
            written += count;
            self.len = self.len.saturating_sub(count);

            if front.offset == front.data.len() {
                self.chunks.pop_front();
            }
        }

        written
    }

    pub fn peek_prefix(&self, output: &mut [u8]) -> usize {
        let mut written = 0;

        for chunk in &self.chunks {
            if written == output.len() {
                break;
            }

            let available = &chunk.data[chunk.offset..];
            let count = available.len().min(output.len() - written);

            output[written..written + count]
                .copy_from_slice(&available[..count]);

            written += count;
        }

        written
    }
}

impl Default for ByteQueue {
    fn default() -> Self {
        Self::new()
    }
}
