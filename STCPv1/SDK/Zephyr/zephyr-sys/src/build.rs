use std::env;
use std::path::PathBuf;

fn main() {
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .clang_arg("-I$ZEPHYR_BASE/include") // oletetaan että ZEPHYR_BASE on asetettu
        .allowlist_function("socket")
        .allowlist_function("bind")
        .allowlist_function("listen")
        .allowlist_function("accept")
        .allowlist_function("recv")
        .allowlist_function("send")
        .allowlist_function("setsockopt")
        .allowlist_type("sockaddr")
        .allowlist_type("sockaddr_in")
        .allowlist_var("AF_INET")
        .allowlist_var("SOCK_STREAM")
        .allowlist_var("IPPROTO_TCP")
        .allowlist_var("SOL_SOCKET")
        .allowlist_var("SO_REUSEADDR")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    println!("cargo:rerun-if-changed=wrapper.h");
}
