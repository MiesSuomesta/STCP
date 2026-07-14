use crate::{error::StcpError, state::{StcpContext,StcpState}};

/* Platform transport hooks. The application used libc send/recv here.
 * Kernel version must call an underlying carrier implementation or enqueue
 * frames to a kernel networking backend. */
pub fn bind(c:&mut StcpContext,a:u32,p:u16)->Result<(),StcpError>{if c.state!=StcpState::New{return Err(StcpError::InvalidState)};c.local_addr=a;c.local_port=p;c.state=StcpState::Bound;Ok(())}
pub fn listen(c:&mut StcpContext,_b:i32)->Result<(),StcpError>{if c.state!=StcpState::Bound{return Err(StcpError::InvalidState)};c.state=StcpState::Listening;Ok(())}
pub fn connect(c:&mut StcpContext,a:u32,p:u16)->Result<(),StcpError>{if c.state!=StcpState::New{return Err(StcpError::InvalidState)};c.peer_addr=a;c.peer_port=p;c.state=StcpState::Handshake;/* run symmetric handshake via carrier */c.state=StcpState::Ready;Ok(())}
pub fn send(c:&mut StcpContext,_d:&[u8])->Result<usize,StcpError>{if c.state!=StcpState::Ready{return Err(StcpError::InvalidState)};Err(StcpError::Again)}
pub fn recv(c:&mut StcpContext,_d:&mut[u8])->Result<usize,StcpError>{if c.state!=StcpState::Ready{return Err(StcpError::InvalidState)};Err(StcpError::Again)}
