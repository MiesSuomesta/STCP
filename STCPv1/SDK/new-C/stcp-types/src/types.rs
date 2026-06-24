
pub type StcpResult<T> = core::result::Result<T, StcpError>;

#[repr(C)]
pub enum StcpError {
    ReadFailed,
    WriteFailed,
    InvalidInput,
}

pub type ServerMessageProcessCB = extern "C" fn(
    input_ptr: *const u8,
    input_len: usize,
    output_buf: *mut u8,
    max_output_len: usize,
    actual_output_len: *mut usize,
);


pub fn force_link_globals_types() {
    globals::zephyr_allocator::touchme_alloc();
//    globals::panic::touchme_panic();
}

