#[cfg(not(feature = "zephyr"))]
mod linux;

#[cfg(feature = "zephyr")]
mod zephyr;

#[cfg(not(feature = "zephyr"))]
pub use linux::StcpListener;

#[cfg(feature = "zephyr")]
pub use zephyr::StcpListener;
