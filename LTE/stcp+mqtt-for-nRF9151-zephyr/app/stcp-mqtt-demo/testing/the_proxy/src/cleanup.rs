use std::ffi::c_void;

pub fn cleanup_connection(
    transport: *mut c_void
) {
    println!(
        "Cleaning connection {:?}",
        transport
    );
}
