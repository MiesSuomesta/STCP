use crate::{error::StcpError, state::StcpContext};

pub fn bind(_ctx: &mut StcpContext, _addr: u32, _port: u16) -> Result<(), StcpError> { Ok(()) }
pub fn listen(_ctx: &mut StcpContext, _backlog: i32) -> Result<(), StcpError> { Ok(()) }
pub fn connect(_ctx: &mut StcpContext, _addr: u32, _port: u16) -> Result<(), StcpError> { Ok(()) }
pub fn send(_ctx: &mut StcpContext, _data: &[u8]) -> Result<usize, StcpError> { Err(StcpError::Again) }
pub fn recv(_ctx: &mut StcpContext, _out: &mut [u8]) -> Result<usize, StcpError> { Err(StcpError::Again) }
pub fn shutdown(_ctx: &mut StcpContext, _how: i32) {}
