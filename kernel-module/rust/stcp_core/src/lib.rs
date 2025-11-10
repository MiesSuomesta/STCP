#![no_std]
extern crate alloc;
pub mod error;
pub mod types;
pub mod crypto;
pub mod handshake;
pub mod packet;

pub use types::{StcpCtx, StcpHandshake};

#[repr(C)]
pub struct StcpSocketState {
    pub handshake_done: bool,
    pub key_id: u64,
}

impl StcpSocketState {
    pub const fn new() -> Self {
        Self {
            handshake_done: false,
            key_id: 0,
        }
    }
}

/// Yksinkertainen handshake-API: tätä kernel-/Zephyr-adapteri kutsuu.
///
/// Tässä nyt placeholder (echo + state flip).
pub fn handshake_process(
    st: &mut StcpSocketState,
    input: &[u8],
    output: &mut [u8],
) -> Result<usize, i32> {
    use error::{EINVAL, EOPNOTSUPP};

    if input.is_empty() || output.is_empty() {
        return Err(EINVAL);
    }

    if !st.handshake_done {
        let to_copy = core::cmp::min(input.len(), output.len());
        output[..to_copy].copy_from_slice(&input[..to_copy]);
        st.handshake_done = true;
        st.key_id = 1;
        Ok(to_copy)
    } else {
        Err(EOPNOTSUPP)
    }
}

