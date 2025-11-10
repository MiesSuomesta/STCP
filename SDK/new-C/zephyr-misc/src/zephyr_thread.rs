extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use core::mem::MaybeUninit;
use iowrapper::stream::StcpStream;
use iowrapper::types::{ServerThreadContext};
use stcptypes::types::*;
use spin::Mutex;
use core::ffi::c_void;
use crate::server_connection::handle_client_connection;

const STACK_SIZE: usize = 2048;

#[repr(C)]
pub struct RustSpawnContext {
    pub thread_ptr: *mut c_void,
    pub stack_ptr: *mut u8,
    pub stack_size: usize,
    pub entry: extern "C" fn(*mut c_void),
    pub arg: *mut c_void,
}

extern "C" {
    pub fn spawn_with_arg(ctx: *mut RustSpawnContext);
}

extern "C" fn server_thread_entry(ctx_ptr: *mut c_void) {
    unsafe {
        let ctx: Box<ServerThreadContext> = Box::from_raw(ctx_ptr.cast());
        handle_client_connection(ctx.stream, ctx.cb);
    }
}

pub fn spawn_handler(stream: Mutex<StcpStream>, cb: ServerMessageProcessCB) {

    // Allokoi Zephyrin säikeen hallintorakenne
    let thread_box = Box::new(MaybeUninit::<[usize; 64]>::uninit());
    let thread_ptr = Box::into_raw(thread_box) as *mut c_void;

    // Allokoi pino
    let mut stack = Vec::with_capacity(STACK_SIZE).into_boxed_slice();
    let stack_ptr = stack.as_mut_ptr();

    // Luo ServerThreadContext oikealla tyypillä
    let ctx = Box::new(ServerThreadContext {
        stream: stream,
        cb,
    });
    let ctx_ptr = Box::into_raw(ctx) as *mut c_void;

    // Rakenna spawn-context
    let spawn_ctx = Box::new(RustSpawnContext {
        thread_ptr,
        stack_ptr,
        stack_size: STACK_SIZE,
        entry: server_thread_entry,
        arg: ctx_ptr,
    });

    // Käynnistä säie
    unsafe {
        spawn_with_arg(Box::into_raw(spawn_ctx));
    }
}
