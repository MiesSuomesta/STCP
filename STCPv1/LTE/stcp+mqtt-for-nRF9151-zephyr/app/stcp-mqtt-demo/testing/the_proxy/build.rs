fn main() {

    cc::Build::new()
        .file("src/c/rust_log_with_printk.c")
        .file("src/c/misc.c")
        .file("src/c/stcp_tcp_recv.c")
        .file("src/c/stcp_tcp_send.c")
        .compile("stcp_linux_shim");

    cc::Build::new()
        .file("src/c/crypto.c")
        .compile("stcp_linux_crypto_shim");

    cc::Build::new()
        .file("src/c/kernel_socket.c")
        .compile("stcp_linux_kernel_socket_shim");

    println!("cargo:rustc-link-lib=crypto");
}
