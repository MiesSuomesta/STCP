pub mod zephyr_thread;
pub mod server_connection;
pub mod dnshelper;

#[macro_use]
extern crate debug;

#[repr(C)]
pub struct RustSpawnContext {
    pub thread_ptr: *mut core::ffi::c_void, // osoittaa Zephyrin `struct k_thread`
    pub stack_ptr: *mut u8,                 // osoittaa stackiin
    pub stack_size: usize,
    pub entry: extern "C" fn(*mut core::ffi::c_void),
    pub arg: *mut core::ffi::c_void,
}

extern "C" {
    pub fn spawn_with_arg(ctx: *mut RustSpawnContext);
    pub fn zephyr_rand_byte() -> u8;
}

#[no_mangle]
pub extern "C" fn __getrandom_custom(buf: *mut u8, len: usize) -> i32 {
    unsafe {
        for i in 0..len {
            *buf.add(i) = zephyr_rand_byte(); // korvaa oikealla Zephyr-funktiolla
        }
    }
    0
}

