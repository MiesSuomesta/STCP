use core::slice;
use crate::types::STCP_MAX_TCP_PAYLOAD_SIZE;

#[derive(Debug)]
pub enum StcpError {
    NullPointer,
    LengthTooBig { len: usize, max_len: usize },
}

const STCP_MAX_TCP_PAYLOAD_SIZE_USIZE: usize = STCP_MAX_TCP_PAYLOAD_SIZE as usize;

/// Luo &mut [u8] –slice osoitteesta ilman mitään debug-tulostusta.
///
/// - `data`: raaka osoitin (userland/kernel buffer)
/// - `len`: haluttu pituus
/// - `param_max_len`: kutsujan mukaan “todellinen bufferin koko”
pub fn stcp_make_mut_slice<'a>(
    data: *mut u8,
    len: usize,
    param_max_len: usize,
) -> Result<&'a mut [u8], StcpError> {
    // 1) Null-check
    if data.is_null() {
        return Err(StcpError::NullPointer);
    }

    // 2) Rajoitetaan maksimipituus sekä kutsujan parametriin että globaaliseen maksimiin
    let max_len = core::cmp::min(param_max_len, STCP_MAX_TCP_PAYLOAD_SIZE_USIZE);

    // Jos pyydetty len > max_len, ilmoitetaan virheestä siististi
    if len > max_len {
        return Err(StcpError::LengthTooBig { len, max_len });
    }

    // 3) Käytettävä pituus (varmuuden vuoksi min)
    let eff_len = core::cmp::min(len, max_len);

    // 4) Varsinainen unsafe – ei mitään debug-tulostusta tänne
    let slice = unsafe { slice::from_raw_parts_mut(data, eff_len) };

    Ok(slice)
}
