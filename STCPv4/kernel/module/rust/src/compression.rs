use alloc::vec::Vec;

use crate::error::StcpError;

pub const FLAG_COMPRESSED: u8 = 0x01;
pub const FLAG_COMPRESSION_CAPABLE: u8 = 0x02;
pub const DEFAULT_COMPRESSION_THRESHOLD: usize = 1024;
const HASH_BITS: usize = 12;
const HASH_SIZE: usize = 1 << HASH_BITS;
const MIN_MATCH: usize = 4;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CompressionMode {
    Off,
    Auto,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum CompressionLevel {
    VeryFast = 1,
    Fast = 2,
    Default = 3,
    High = 4,
    VeryHigh = 5,
}

impl CompressionLevel {
    pub fn from_u32(level: u32) -> Option<Self> {
        match level {
            1 => Some(Self::VeryFast),
            2 => Some(Self::Fast),
            3 => Some(Self::Default),
            4 => Some(Self::High),
            5 => Some(Self::VeryHigh),
            _ => None,
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct CompressionConfig {
    pub mode: CompressionMode,
    pub threshold: usize,
    pub level: CompressionLevel,
}

impl Default for CompressionConfig {
    fn default() -> Self {
        Self {
            mode: CompressionMode::Off,
            threshold: DEFAULT_COMPRESSION_THRESHOLD,
            level: CompressionLevel::Default,
        }
    }
}

impl CompressionConfig {
    #[inline]
    pub fn should_try(self, len: usize) -> bool {
        self.mode == CompressionMode::Auto && len >= self.threshold
    }
}

#[inline]
fn hash4(bytes: &[u8], pos: usize) -> usize {
    let value = u32::from_le_bytes([
        bytes[pos], bytes[pos + 1], bytes[pos + 2], bytes[pos + 3],
    ]);
    ((value.wrapping_mul(0x9E37_79B1)) >> (32 - HASH_BITS)) as usize
}

fn push_len(out: &mut Vec<u8>, mut len: usize) -> Result<(), StcpError> {
    while len >= 255 {
        out.try_reserve(1).map_err(|_| StcpError::NoMem)?;
        out.push(255);
        len -= 255;
    }
    out.try_reserve(1).map_err(|_| StcpError::NoMem)?;
    out.push(len as u8);
    Ok(())
}

fn emit_sequence(
    out: &mut Vec<u8>,
    literals: &[u8],
    offset: usize,
    match_len: usize,
) -> Result<(), StcpError> {
    let literal_len = literals.len();
    let match_field = match_len - MIN_MATCH;
    let token = ((literal_len.min(15) as u8) << 4) | (match_field.min(15) as u8);

    out.try_reserve(1 + literal_len + 2 + 8)
        .map_err(|_| StcpError::NoMem)?;
    out.push(token);
    if literal_len >= 15 {
        push_len(out, literal_len - 15)?;
    }
    out.extend_from_slice(literals);
    out.extend_from_slice(&(offset as u16).to_le_bytes());
    if match_field >= 15 {
        push_len(out, match_field - 15)?;
    }
    Ok(())
}

fn emit_last_literals(out: &mut Vec<u8>, literals: &[u8]) -> Result<(), StcpError> {
    let literal_len = literals.len();
    out.try_reserve(1 + literal_len + 8)
        .map_err(|_| StcpError::NoMem)?;
    out.push((literal_len.min(15) as u8) << 4);
    if literal_len >= 15 {
        push_len(out, literal_len - 15)?;
    }
    out.extend_from_slice(literals);
    Ok(())
}

/// Small no_std LZ4 block encoder. The output is a raw LZ4 block with no frame
/// wrapper; STCP carries the original length separately in a compact varint.
pub fn compress_block(src: &[u8], level: CompressionLevel) -> Result<Vec<u8>, StcpError> {
    match level {
        CompressionLevel::Default => compress_block_default(src),
        CompressionLevel::VeryFast => compress_block_fast(src, 4),
        CompressionLevel::Fast => compress_block_fast(src, 2),
        CompressionLevel::High => compress_block_deep(src, 4),
        CompressionLevel::VeryHigh => compress_block_deep(src, 16),
    }
}

/// Level 3. Keep this encoder byte-for-byte equivalent to the original STCPv3
/// compressor so DEFAULT remains the historical benchmark baseline.
fn compress_block_default(src: &[u8]) -> Result<Vec<u8>, StcpError> {
    if src.is_empty() {
        return Ok(Vec::new());
    }

    let mut table = Vec::new();
    table.try_reserve_exact(HASH_SIZE).map_err(|_| StcpError::NoMem)?;
    table.resize(HASH_SIZE, 0u32);

    let mut out = Vec::new();
    out.try_reserve_exact(src.len()).map_err(|_| StcpError::NoMem)?;

    let mut anchor = 0usize;
    let mut pos = 0usize;

    while pos + MIN_MATCH <= src.len() {
        let h = hash4(src, pos);
        let stored = table[h];
        table[h] = (pos as u32).saturating_add(1);

        if stored != 0 {
            let candidate = stored as usize - 1;
            let distance = pos - candidate;
            if distance <= u16::MAX as usize &&
               candidate + MIN_MATCH <= src.len() &&
               src[candidate..candidate + MIN_MATCH] == src[pos..pos + MIN_MATCH]
            {
                let mut match_len = MIN_MATCH;
                while pos + match_len < src.len() &&
                      src[candidate + match_len] == src[pos + match_len]
                {
                    match_len += 1;
                }

                emit_sequence(&mut out, &src[anchor..pos], distance, match_len)?;
                pos += match_len;
                anchor = pos;
                continue;
            }
        }

        pos += 1;
    }

    emit_last_literals(&mut out, &src[anchor..])?;
    Ok(out)
}

/// Levels 1-2 trade ratio for fewer hash probes. The wire format remains the
/// same raw LZ4 block; only the encoder search effort changes.
fn compress_block_fast(src: &[u8], skip: usize) -> Result<Vec<u8>, StcpError> {
    if src.is_empty() { return Ok(Vec::new()); }
    let mut table = Vec::new();
    table.try_reserve_exact(HASH_SIZE).map_err(|_| StcpError::NoMem)?;
    table.resize(HASH_SIZE, 0u32);
    let mut out = Vec::new();
    out.try_reserve_exact(src.len()).map_err(|_| StcpError::NoMem)?;
    let mut anchor = 0usize;
    let mut pos = 0usize;

    while pos + MIN_MATCH <= src.len() {
        let h = hash4(src, pos);
        let stored = table[h];
        table[h] = (pos as u32).saturating_add(1);
        if stored != 0 {
            let candidate = stored as usize - 1;
            let distance = pos - candidate;
            if distance <= u16::MAX as usize &&
               src[candidate..candidate + MIN_MATCH] == src[pos..pos + MIN_MATCH] {
                let mut match_len = MIN_MATCH;
                while pos + match_len < src.len() &&
                      src[candidate + match_len] == src[pos + match_len] { match_len += 1; }
                emit_sequence(&mut out, &src[anchor..pos], distance, match_len)?;
                pos += match_len;
                anchor = pos;
                continue;
            }
        }
        pos = pos.saturating_add(skip);
    }
    emit_last_literals(&mut out, &src[anchor..])?;
    Ok(out)
}

/// Levels 4-5 keep a short hash chain and choose the longest match among more
/// candidates. Decoder and wire format are unchanged.
fn compress_block_deep(src: &[u8], max_candidates: usize) -> Result<Vec<u8>, StcpError> {
    if src.is_empty() { return Ok(Vec::new()); }
    let mut head = Vec::new();
    head.try_reserve_exact(HASH_SIZE).map_err(|_| StcpError::NoMem)?;
    head.resize(HASH_SIZE, 0u32);
    let mut prev = Vec::new();
    prev.try_reserve_exact(src.len()).map_err(|_| StcpError::NoMem)?;
    prev.resize(src.len(), 0u32);
    let mut out = Vec::new();
    out.try_reserve_exact(src.len()).map_err(|_| StcpError::NoMem)?;
    let mut anchor = 0usize;
    let mut pos = 0usize;

    while pos + MIN_MATCH <= src.len() {
        let h = hash4(src, pos);
        let mut stored = head[h];
        prev[pos] = stored;
        head[h] = (pos as u32).saturating_add(1);
        let mut best_candidate = 0usize;
        let mut best_len = 0usize;
        let mut checked = 0usize;
        while stored != 0 && checked < max_candidates {
            let candidate = stored as usize - 1;
            if candidate >= pos { break; }
            let distance = pos - candidate;
            if distance > u16::MAX as usize { break; }
            if candidate + MIN_MATCH <= src.len() &&
               src[candidate..candidate + MIN_MATCH] == src[pos..pos + MIN_MATCH] {
                let mut match_len = MIN_MATCH;
                while pos + match_len < src.len() &&
                      src[candidate + match_len] == src[pos + match_len] { match_len += 1; }
                if match_len > best_len {
                    best_len = match_len;
                    best_candidate = candidate;
                }
            }
            stored = prev[candidate];
            checked += 1;
        }
        if best_len >= MIN_MATCH {
            emit_sequence(&mut out, &src[anchor..pos], pos - best_candidate, best_len)?;
            pos += best_len;
            anchor = pos;
            continue;
        }
        pos += 1;
    }
    emit_last_literals(&mut out, &src[anchor..])?;
    Ok(out)
}

fn read_extended_len(src: &[u8], input: &mut usize, base: usize) -> Result<usize, StcpError> {
    let mut len = base;
    loop {
        let value = *src.get(*input).ok_or(StcpError::Protocol)? as usize;
        *input += 1;
        len = len.checked_add(value).ok_or(StcpError::Protocol)?;
        if value != 255 {
            return Ok(len);
        }
    }
}

pub fn decompress_block(src: &[u8], original_len: usize) -> Result<Vec<u8>, StcpError> {
    let mut out = Vec::new();
    out.try_reserve_exact(original_len).map_err(|_| StcpError::NoMem)?;
    let mut input = 0usize;

    while input < src.len() {
        let token = src[input];
        input += 1;

        let mut literal_len = (token >> 4) as usize;
        if literal_len == 15 {
            literal_len = read_extended_len(src, &mut input, 15)?;
        }
        let literal_end = input.checked_add(literal_len).ok_or(StcpError::Protocol)?;
        if literal_end > src.len() || out.len().checked_add(literal_len).ok_or(StcpError::Protocol)? > original_len {
            return Err(StcpError::Protocol);
        }
        out.extend_from_slice(&src[input..literal_end]);
        input = literal_end;

        if input == src.len() {
            break;
        }

        if input + 2 > src.len() {
            return Err(StcpError::Protocol);
        }
        let offset = u16::from_le_bytes([src[input], src[input + 1]]) as usize;
        input += 2;
        if offset == 0 || offset > out.len() {
            return Err(StcpError::Protocol);
        }

        let mut match_len = (token & 0x0f) as usize + MIN_MATCH;
        if (token & 0x0f) == 15 {
            match_len = read_extended_len(src, &mut input, 15 + MIN_MATCH)?;
        }
        if out.len().checked_add(match_len).ok_or(StcpError::Protocol)? > original_len {
            return Err(StcpError::Protocol);
        }

        let start = out.len() - offset;
        for i in 0..match_len {
            let byte = out[start + i];
            out.push(byte);
        }
    }

    if out.len() != original_len {
        return Err(StcpError::Protocol);
    }
    Ok(out)
}

pub fn encode_uvarint(mut value: usize, out: &mut [u8; 10]) -> usize {
    let mut n = 0usize;
    loop {
        let mut byte = (value & 0x7f) as u8;
        value >>= 7;
        if value != 0 {
            byte |= 0x80;
        }
        out[n] = byte;
        n += 1;
        if value == 0 {
            return n;
        }
    }
}

pub fn decode_uvarint(input: &[u8]) -> Result<(usize, usize), StcpError> {
    let mut value = 0usize;
    let mut shift = 0usize;
    for (index, byte) in input.iter().copied().enumerate().take(10) {
        let part = (byte & 0x7f) as usize;
        value |= part.checked_shl(shift as u32).ok_or(StcpError::Protocol)?;
        if byte & 0x80 == 0 {
            return Ok((value, index + 1));
        }
        shift += 7;
        if shift >= usize::BITS as usize {
            return Err(StcpError::Protocol);
        }
    }
    Err(StcpError::Protocol)
}
