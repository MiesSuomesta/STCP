
extern crate alloc;
use alloc::format;

#[macro_export]
macro_rules! zprint {
    ($fmt:literal $(, $arg:expr)* $(,)?) => {{
        use core::fmt::Write;
        struct Writer;

        impl Write for Writer {
            fn write_str(&mut self, s: &str) -> core::fmt::Result {
                unsafe {

                    extern "C" {
                        pub fn printk(fmt: *const u8, ...) -> i32;
                    }

                    for b in s.bytes() {
                        printk(b"%c\0".as_ptr(), b as i32);
                    }
                }
                Ok(())
            }
        }

        let _ = write!(Writer, $fmt $(, $arg)*);
    }};
}

#[macro_export]
macro_rules! dbg {
    ($($arg:tt)*) => {
        {
            let formatted = format!($($arg)*);
            zprint!("[{}:{}] {}", file!(), line!(), formatted);
        }
    };
}
