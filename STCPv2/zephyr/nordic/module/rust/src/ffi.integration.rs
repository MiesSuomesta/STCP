/*
 * Add to ffi.rs:
 */

#[unsafe(no_mangle)]
pub extern "C" fn stcp_rust_set_carrier(
    raw: *mut c_void,
    carrier: *mut c_void,
) {
    let _ = with_ctx(raw, |ctx| {
        crate::state::set_carrier(
            ctx,
            carrier as usize,
        );
    });
}
