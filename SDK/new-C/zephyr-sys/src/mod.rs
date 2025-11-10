
#[cfg(feature = "bindgen")] // sallii tämän vain jos käytät build.rs
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(not(feature = "bindgen"))]
pub mod net {

    pub type socklen_t = u32;

    #[repr(C)]
    pub struct sockaddr {
        pub sa_family: u16,
        pub sa_data: [u8; 14],
    }

    #[repr(C)]
    pub struct in_addr {
        pub s_addr: u32,
    }

    #[repr(C)]
    pub struct sockaddr_in {
        pub sin_family: u16,
        pub sin_port: u16,
        pub sin_addr: in_addr,
        pub sin_zero: [u8; 8],
    }

    extern "C" {
        pub fn stcp_modem_socket(family: i32, socktype: i32, proto: i32) -> i32;
        pub fn stcp_modem_connect(fd: i32, addr: *const sockaddr, addrlen: socklen_t) -> i32;
        pub fn stcp_modem_send(fd: i32, buf: *const u8, len: usize, flags: i32) -> isize;
        pub fn stcp_modem_recv(fd: i32, buf: *mut u8, len: usize, flags: i32) -> isize;
        pub fn stcp_modem_close(fd: i32) -> i32;
        pub fn stcp_modem_bind(fd: i32, addr: *const sockaddr, len: socklen_t) -> i32;
        pub fn stcp_modem_listen(fd: i32, backlog: i32) -> i32;
        pub fn stcp_modem_accept(fd: i32, addr: *mut sockaddr, len: *mut socklen_t) -> i32;
    }

    pub const AF_INET: i32 = 2;
    pub const SOCK_STREAM: i32 = 1;
    pub const IPPROTO_TCP: i32 = 6;
    pub const SOL_SOCKET: i32 = 1;
    pub const SO_REUSEADDR: i32 = 2;

}

//use globals::panic::panic as _;
