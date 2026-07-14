pub mod crypto;
pub mod error;
pub mod packet;
pub mod socket;
pub mod logic;
pub mod types;
pub mod io;

pub use error::StcpError;
pub use packet::{StcpHeader, StcpPacketType, STCP_HEADER_LEN, STCP_MAGIC, STCP_PUBLIC_KEY_LEN};
pub use types::{StcpContext, StcpState};

pub use socket::{
    stcp_accept, 
    stcp_bind, 
    stcp_connect, 
    stcp_listen,
    stcp_recv, 
    stcp_send, 
    stcp_socket
};

