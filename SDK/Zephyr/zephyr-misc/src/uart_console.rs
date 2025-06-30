extern crate alloc;
use alloc::vec::Vec;
use alloc::boxed::Box;

use core::fmt::{self, Write};

use stcpdefines::defines::*;
use stcptypes::types::*;
use crate::stcp_stream::StcpStream;

pub struct UartWriter;

extern "C" {
    fn uart_send_byte(byte: u8);
}

impl Write for UartWriter {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for byte in s.as_bytes() {
            unsafe {
                uart_send_byte(*byte);
            }
        }
        Ok(())
    }
}

pub struct UartStream;

extern "C" {
    fn uart_recv(buf: *mut u8, maxlen: usize) -> usize;
    fn uart_send(buf: *const u8, len: usize) -> usize;
}

impl StcpStream for UartStream {

    fn write(&mut self, buf: &[u8]) -> Result<(), ()> {
        let sent = unsafe { uart_send(buf.as_ptr(), buf.len()) };
        if sent == buf.len() { Ok(()) } else { Err( () ) }
    }

    fn write_all(&mut self, buf: &[u8]) -> Result<(), ()> {
        let sent = unsafe { uart_send(buf.as_ptr(), buf.len()) };
        if sent == buf.len() { Ok(()) } else { Err( () ) }
    }

    fn read(&mut self, buf: &mut [u8]) -> Result<usize, ()> {
        let read = unsafe { uart_recv(buf.as_mut_ptr(), buf.len()) };
        if read > 0 { Ok(read) } else { Err( () ) }
    }
}


