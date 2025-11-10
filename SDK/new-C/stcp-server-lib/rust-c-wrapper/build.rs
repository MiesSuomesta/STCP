use std::env;
use std::path::PathBuf;
use std::fs;

fn main() {
    let crate_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    //let target = env::var("TARGET").unwrap();

    // 🔗 Linkitä staattinen kirjasto oikein MUSL-buildiä varten
/*    let lib_path = crate_dir
        .join("../../stcp-server-lib/rust-server-lib")
        .join("target")
        .join(&target)
        .join("release");
    println!("cargo:rustc-link-search=native={}", lib_path.display());
    println!("cargo:rustc-link-lib=static=stcpserverlib");
*/
    // 📄 Generoi C-header tiedosto
    let config_path = crate_dir.join("cbindgen.toml");
    let out_path = crate_dir.join("include").join("stcp_server_cwrapper_lib.h");

    fs::create_dir_all(out_path.parent().unwrap()).unwrap();

    cbindgen::Builder::new()
        .with_crate(crate_dir.clone())
        .with_config(cbindgen::Config::from_file(config_path).unwrap())
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(out_path);

    // 🔄 Rebuild triggerit
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=cbindgen.toml");
    println!("cargo:rerun-if-changed=build.rs");
}

