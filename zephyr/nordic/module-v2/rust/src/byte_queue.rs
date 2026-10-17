use alloc::{
    collections::VecDeque,
    vec::Vec,
};

use crate::error::StcpError;

pub const BYTE_QUEUE_CHUNK_SIZE: usize = 1152;

// Per-session ceilings. These are admission limits, not preallocated pools.
// Kernel(-105) is ENOBUFS: distinct from allocator ENOMEM (-12).
pub const RX_WIRE_BUDGET: usize = 16 * 1024;
pub const RX_APP_BUDGET: usize = 16 * 1024;
pub const RX_REORDER_BUDGET: usize = 8 * 1024;
pub const RX_CHUNK_LIMIT: usize = 64;
pub const RX_FRAME_DATA_LIMIT: usize = 4096;

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

    /// Backing capacity, including consumed prefixes and spare tail space.
    pub fn retained_bytes(&self) -> usize {
        self.chunks.iter().fold(0usize, |sum, c| sum.saturating_add(c.data.capacity()))
    }

    fn admit_owned(&self, input: &Vec<u8>, byte_limit: usize, chunk_limit: usize) -> Result<(), StcpError> {
        if self.chunks.len() >= chunk_limit || input.capacity() > byte_limit.saturating_sub(self.retained_bytes()) {
            return Err(StcpError::Kernel(-105));
        }
        Ok(())
    }

    pub fn push_slice(&mut self, input: &[u8]) -> Result<(), StcpError> {
        self.push_slice_bounded(input, usize::MAX, usize::MAX)
    }

    pub fn push_slice_bounded(
        &mut self,
        mut input: &[u8],
        byte_limit: usize,
        chunk_limit: usize,
    ) -> Result<(), StcpError> {
        let added = input.len();
        let new_len = self.len.checked_add(added).ok_or(StcpError::NoMem)?;

        if new_len > byte_limit { return Err(StcpError::Kernel(-105)); }
        // Roll back appended chunks and the old tail on any allocation failure.
        // Spare capacity may grow, but logical contents and offsets stay unchanged.
        let old_chunks = self.chunks.len();
        let old_tail_len = self.chunks.back().map(|chunk| chunk.data.len());
        let result = (|| -> Result<(), StcpError> {
        while !input.is_empty() {
            let tail_budget = byte_limit.saturating_sub(self.retained_bytes());
            if let Some(back) = self.chunks.back_mut() {
                let spare = BYTE_QUEUE_CHUNK_SIZE.saturating_sub(back.data.len());

                if spare != 0 {
                    let count = spare.min(input.len());
                    // Account for existing capacity without charging tail reuse twice.
                    let extra = back.data.len().saturating_add(count).saturating_sub(back.data.capacity());
                    // Sum other chunks before obtaining the mutable tail below.
                    if extra > tail_budget { return Err(StcpError::Kernel(-105)); }
                    back.data
                        .try_reserve_exact(count)
                        .map_err(|_| StcpError::NoMem)?;
                    back.data.extend_from_slice(&input[..count]);
                    input = &input[count..];
                    continue;
                }
            }

            let remaining = byte_limit.saturating_sub(self.retained_bytes());
            let count = input.len().min(BYTE_QUEUE_CHUNK_SIZE).min(remaining);
            if count == 0 || self.chunks.len() >= chunk_limit { return Err(StcpError::Kernel(-105)); }
            let mut chunk = Vec::new();
            /* Allocate the bytes actually received, including their wire
             * header. CHUNK_SIZE is a maximum, not a minimum allocation. */
            chunk
                .try_reserve_exact(count)
                .map_err(|_| StcpError::NoMem)?;
            chunk.extend_from_slice(&input[..count]);
            self.chunks.try_reserve(1).map_err(|_| StcpError::NoMem)?;
            self.chunks.push_back(ByteChunk { data: chunk, offset: 0 });
            input = &input[count..];
        }

            Ok(())
        })();
        if let Err(error) = result {
            self.chunks.truncate(old_chunks);
            if let (Some(tail), Some(old_len)) = (self.chunks.back_mut(), old_tail_len) {
                tail.data.truncate(old_len);
            }
            return Err(error);
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
        let new_len = self.len.checked_add(count).ok_or(StcpError::NoMem)?;
        self.chunks.try_reserve(1).map_err(|_| StcpError::NoMem)?;
        self.len = new_len;
        self.chunks.push_back(ByteChunk {
            data: input,
            offset: 0,
        });
        Ok(())
    }

    /// Append a range from an owned buffer without copying. Bytes before
    /// `offset` remain allocated but are skipped by the queue reader.
    pub fn push_vec_from(&mut self, input: Vec<u8>, offset: usize) -> Result<(), StcpError> {
        self.push_vec_from_bounded(input, offset, usize::MAX, usize::MAX)
    }

    pub fn push_vec_from_bounded(
        &mut self,
        input: Vec<u8>,
        offset: usize,
        byte_limit: usize,
        chunk_limit: usize,
    ) -> Result<(), StcpError> {
        if offset > input.len() {
            return Err(StcpError::Protocol);
        }
        let count = input.len() - offset;
        if count == 0 {
            return Ok(());
        }
        self.admit_owned(&input, byte_limit, chunk_limit)?;
        let new_len = self.len.checked_add(count).ok_or(StcpError::NoMem)?;
        self.chunks.try_reserve(1).map_err(|_| StcpError::NoMem)?;
        self.len = new_len;
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

    /// Consume a prefix and return the following payload atomically on ENOMEM.
    pub fn take_after_prefix(&mut self, prefix: usize, count: usize) -> Result<Vec<u8>, StcpError> {
        let total = prefix.checked_add(count).ok_or(StcpError::Protocol)?;
        if self.len < total { return Err(StcpError::Again); }
        if prefix == 0 { return self.take_or_read_vec(count); }
        // Preserve the zero-copy path for a separate header and owned payload.
        let exact = self.chunks.front().map(|c| c.data.len() - c.offset == prefix).unwrap_or(false)
            && self.chunks.get(1).map(|c| c.offset == 0 && c.data.len() == count).unwrap_or(false);
        if exact {
            self.discard(prefix);
            return self.take_or_read_vec(count);
        }
        let mut output = Vec::new();
        output.try_reserve_exact(count).map_err(|_| StcpError::NoMem)?;
        output.resize(count, 0);
        // No fallible operation remains after consuming the header.
        self.discard(prefix);
        self.read_into(&mut output);
        Ok(output)
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
