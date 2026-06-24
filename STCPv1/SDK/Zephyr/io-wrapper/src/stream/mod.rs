// cargo build --features zephyr
#[cfg(not(feature = "zephyr"))]
mod linux;

#[cfg(feature = "zephyr")]
mod zephyr;

#[cfg(not(feature = "zephyr"))]
pub use linux::StcpStream;

#[cfg(feature = "zephyr")]
pub use zephyr::StcpStream;
