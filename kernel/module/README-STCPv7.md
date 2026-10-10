# STCPv7 first optimization — 2026-10-10

Baseline: module-10102026_093325.zip and benchmark-results-10102026_094838.zip.
Original upload and extracted baseline were not modified.

Changes:
- Compress directly after the existing original-length varint. Removes the second compressed buffer allocation and the full compressed-result copy.
- Decode LZ4 matches with safe bulk copies. Handles overlapping matches by expanding initialized source ranges, including offset=1.
- Preserve socket API, encryption, packet layout, compression levels, fallback decision and compression counter semantics.

Apply to the module directory (the directory containing rust/ and src/):

    patch --dry-run -p1 < /path/to/STCPv7-optimization.patch
    patch -p1 < /path/to/STCPv7-optimization.patch

Alternatively copy rust/src/compression.rs and rust/src/session.rs from this package to the matching module directory. The package contains the source files supplied in the original archive; it does not add missing build infrastructure.

Validation:
- Independent bulk-copy comparison against the original byte loop: 1290 offset/length combinations PASS.
- Rust compiler and kernel build tree unavailable in the creation environment. Rust compilation, linked kernel module, interoperability and performance NOT verified here.
- Run tests/run-compression-check.sh with rustc installed. It compares all five levels against the preserved original encoder, prefix layout and decoding in both directions, including empty, random and periodic inputs and malformed blocks.
- Rebuild/install with the existing project workflow, then repeat the same binary-corpus and compression-levels benchmark matrix. Required baseline: 406/406 and 696/696 PASS. Compare RX timing, CPU and unchanged compressed byte counts before accepting.
- No speedup claimed before those measurements. TX compressed buffers still allocate per frame; this change removes one allocation/copy, not all allocations.
- tx_wire_bytes retains its existing compression-plaintext meaning; it is not a physical wire-byte counter.

Rollback: reverse the patch with patch -R -p1, or restore the original archive.
