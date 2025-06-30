#![no_std]
#![no_main]
pub mod compile_checks;
pub mod panic;
pub mod zephyr_allocator;

#[used]
static TOUCH: fn() = touchme;

pub fn touchme() {
    // Ei tarvitse tehdä mitään — pakottaa linkityksen
}
