fn main() {

    cc::Build::new()
        .file("src/c/linux-shim.c")
        .compile("stcp_linux_shim");

    cc::Build::new()
        .file("src/c/crypto.c")
        .compile("stcp_linux_crypto_shim");

    println!("cargo:rustc-link-lib=crypto");
}
