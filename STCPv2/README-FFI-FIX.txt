STCPv2 LAST_NOMEM_STAGE FFI export fix
======================================

Apply this AFTER stcp-zephyr-last-nomem-stage-overlay.zip.

Extract in repository root:

    cd ~/STCP/STCPv2
    unzip -o stcp-zephyr-last-nomem-stage-ffi-fix-overlay.zip

Changed files:
    kernel/module/rust/src/carrier.rs
    kernel/module/rust/src/ffi.rs

Fix:
    The diagnostic state remains in carrier.rs, but the exported C ABI symbol

        stcp_rust_last_nomem_stage

    now lives in ffi.rs beside the other stcp_rust_* exports.

carrier.rs now provides only:

    pub(crate) fn last_nomem_stage() -> u32

ffi.rs exports:

    #[unsafe(no_mangle)]
    pub extern "C" fn stcp_rust_last_nomem_stage() -> u32

After rebuilding, verify the Rust artifact if desired:

    nm -g <artifact> | grep stcp_rust_last_nomem_stage

Expected:
    T stcp_rust_last_nomem_stage
