extern crate alloc;
use alloc::string::ToString;

use core::ffi::{c_int, c_void};

use crate::error::StcpError;

pub type RawFd = c_int;

type SizeT = usize;
type SSizeT = isize;

const MSG_PEEK: c_int = 0x02;

#[cfg(target_os = "linux")]
const MSG_NOSIGNAL: c_int = 0x4000;

#[cfg(not(target_os = "linux"))]
const MSG_NOSIGNAL: c_int = 0;

unsafe extern "C" {
    fn send(
        sockfd: c_int,
        buf: *const c_void,
        len: SizeT,
        flags: c_int,
    ) -> SSizeT;

    fn recv(
        sockfd: c_int,
        buf: *mut c_void,
        len: SizeT,
        flags: c_int,
    ) -> SSizeT;
}

/*
 * Julkinen API
 */

 #[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct Iovec {
    iov_base: *mut c_void,
    iov_len: SizeT,
}

unsafe extern "C" {
    fn writev(
        fd: c_int,
        iov: *const Iovec,
        iovcnt: c_int,
    ) -> SSizeT;
}

pub fn stcp_send(
    fd: RawFd,
    data: &[u8],
) -> Result<usize, StcpError> {
    raw_socket_send(fd, data)
}

pub fn stcp_recv(
    fd: RawFd,
    out: &mut [u8],
) -> Result<usize, StcpError> {
    raw_socket_recv(fd, out)
}

pub fn stcp_peek(
    fd: RawFd,
    out: &mut [u8],
) -> Result<usize, StcpError> {
    raw_socket_recv_flags(fd, out, MSG_PEEK)
}

/*
 * Raw BSD socket wrappers
 */

pub fn raw_socket_send(
    fd: RawFd,
    data: &[u8],
) -> Result<usize, StcpError> {
    let ret = unsafe {
        send(
            fd,
            data.as_ptr() as *const c_void,
            data.len(),
            MSG_NOSIGNAL,
        )
    };

    if ret < 0 {
        Err(StcpError::Transport("send failed".to_string()))
    } else {
        Ok(ret as usize)
    }
}

pub fn raw_socket_recv_flags(
    fd: RawFd,
    buf: &mut [u8],
    flags: c_int,
) -> Result<usize, StcpError> {
    //println!("[Sock {}] Recv ....", fd);

    let ret = unsafe {
        recv(
            fd,
            buf.as_mut_ptr() as *mut c_void,
            buf.len(),
            flags,
        )
    };

    //println!("[Sock {}] Recv ret {}", fd, ret);

    if ret < 0 {
        return Err(StcpError::Transport(
            "recv failed".to_string(),
        ));
    }

    if ret == 0 {
        return Err(StcpError::Closed(
            "peer closed socket".to_string(),
        ));
    }

    Ok(ret as usize)
}

pub fn raw_socket_recv(
    fd: RawFd,
    buf: &mut [u8],
) -> Result<usize, StcpError> {
    raw_socket_recv_flags(fd, buf, 0)
}

pub fn raw_socket_peek(
    fd: RawFd,
    buf: &mut [u8],
) -> Result<usize, StcpError> {
    raw_socket_recv_flags(fd, buf, MSG_PEEK)
}

pub fn raw_socket_send_exact(
    fd: RawFd,
    mut data: &[u8],
) -> Result<usize, StcpError> {
    let total = data.len();

    while !data.is_empty() {
        let n = raw_socket_send(fd, data)?;

        if n == 0 {
            return Err(StcpError::Closed(
                "send returned 0".to_string(),
            ));
        }

        data = &data[n..];
    }

    Ok(total)
}

pub fn raw_socket_recv_exact(
    fd: RawFd,
    mut out: &mut [u8],
) -> Result<usize, StcpError> {
    let total = out.len();

    while !out.is_empty() {
        let n = raw_socket_recv(fd, out)?;

        if n == 0 {
            return Err(StcpError::Closed(
                "recv returned 0".to_string(),
            ));
        }

        let tmp = out;
        out = &mut tmp[n..];
    }

    Ok(total)
}

pub fn raw_socket_send_vectored_exact(
    fd: RawFd,
    bufs: &[&[u8]],
) -> Result<usize, StcpError> {
    let mut total_sent = 0usize;

    for b in bufs {
        total_sent += b.len();
    }

    let mut iovecs = [
        Iovec {
            iov_base: core::ptr::null_mut(),
            iov_len: 0,
        };
        4
    ];

    if bufs.len() > iovecs.len() {
        return Err(StcpError::Transport(
            "too many iov buffers".to_string(),
        ));
    }

    for (i, b) in bufs.iter().enumerate() {
        iovecs[i] = Iovec {
            iov_base: b.as_ptr() as *mut c_void,
            iov_len: b.len(),
        };
    }

    let mut sent = 0usize;

    while sent < total_sent {
        let ret = unsafe {
            writev(
                fd,
                iovecs.as_ptr(),
                bufs.len() as c_int,
            )
        };

        if ret < 0 {
            return Err(StcpError::Transport(
                "writev failed".to_string(),
            ));
        }

        if ret == 0 {
            return Err(StcpError::Closed(
                "writev returned 0".to_string(),
            ));
        }

        sent += ret as usize;

        // Yksinkertainen fallback: jos partial write tapahtuu,
        // lähetetään loput normaalilla send_exactillä.
        if sent < total_sent {
            let mut flat = alloc::vec::Vec::with_capacity(total_sent - sent);

            let mut skip = sent;
            for b in bufs {
                if skip >= b.len() {
                    skip -= b.len();
                    continue;
                }

                flat.extend_from_slice(&b[skip..]);
                skip = 0;
            }

            raw_socket_send_exact(fd, &flat)?;
            break;
        }
    }

    Ok(total_sent)
}
