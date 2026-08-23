fn main() {
    println!("cargo:rerun-if-changed=../../../module/rust/Cargo.toml");
    println!("cargo:rerun-if-changed=../../../module/rust/src");
}
