
#[macro_export]
macro_rules! dprint {
    ($($arg:tt)*) => {
        println!(
            "📍 [{}:{}] {}",
            file!(),
            line!(),
            format!($($arg)*)
        );
    };
}
