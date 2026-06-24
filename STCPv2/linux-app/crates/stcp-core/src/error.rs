use std::fmt;
use std::io;

#[derive(Debug)]
pub enum StcpError {
    Io(io::Error),
    InvalidState(&'static str),
    Protocol(String),
    Crypto(String),
    Closed(String),
    Transport(String),
    Unsupported(String),
}

impl fmt::Display for StcpError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            StcpError::Io(e) => write!(f, "io error: {e}"),
            StcpError::InvalidState(s) => write!(f, "invalid state: {s}"),
            StcpError::Protocol(s) => write!(f, "protocol error: {s}"),
            StcpError::Crypto(s) => write!(f, "crypto error: {s}"),
            StcpError::Closed(s) => write!(f, "closed: {s}"),
            StcpError::Transport(s) => write!(f, "transport: {s}"),
            StcpError::Unsupported(s) => write!(f, "unsupported: {s}"),
        }
    }
}

impl std::error::Error for StcpError {}

impl From<io::Error> for StcpError {
    fn from(value: io::Error) -> Self {
        StcpError::Io(value)
    }
}
