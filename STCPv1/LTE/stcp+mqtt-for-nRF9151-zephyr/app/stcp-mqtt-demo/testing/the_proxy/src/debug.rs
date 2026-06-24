
unsafe extern "C" {
    pub fn stcp_uptime_ms() -> i64;
}

#[macro_export]
macro_rules! stcp_dbg {
    ($($arg:tt)*) => {{
        let ts =
            unsafe {
                stcp_uptime_ms()
            };

        println!(
            "[{:08} ms] {}",
            ts,
            format_args!($($arg)*)
        );
    }};
}

#[macro_export]
macro_rules! stcp_dbg_nln {
    ($($arg:tt)*) => {{
        let ts =
            unsafe {
                stcp_uptime_ms()
            };

        print!(
            "[{:08} ms] {}",
            ts,
            format_args!($($arg)*)
        );
    }};
}

pub fn log(msg: &str) {
    stcp_dbg!("[PROXY] {}", msg);
}

pub fn stcp_dump_hex(msg: &str, data: &[u8]) {


    stcp_dbg_nln!("Dump [{:?}] ", msg);

    for (i, chunk) in data.chunks(16).enumerate() {

        print!("{:04x}: ", i * 16);

        for b in chunk {
            print!("{:02x} ", b);
        }

        print!("|");

        for b in chunk {
            let c = *b;

            if c.is_ascii_graphic() || c == b' ' {
                print!("{}", c as char);
            } else {
                print!(".");
            }
        }

        println!("|");
    }
}