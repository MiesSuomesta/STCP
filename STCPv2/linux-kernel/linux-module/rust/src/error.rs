#[derive(Debug, Clone, Copy)]
pub enum StcpError {
    InvalidState,
    AddressInUse,
    ConnectionRefused,
    Closed,
    Again,
    NoMem,
    Unsupported,
}

impl StcpError {
    pub const fn errno(self) -> i32 {
        match self {
            Self::InvalidState => -22,
            Self::AddressInUse => -98,
            Self::ConnectionRefused => -111,
            Self::Closed => -107,
            Self::Again => -11,
            Self::NoMem => -12,
            Self::Unsupported => -95,
        }
    }
}
