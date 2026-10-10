extern crate alloc;
#[path = "../rust/src/error.rs"] mod error;
#[path = "compression_baseline.rs"] mod baseline;
#[path = "../rust/src/compression.rs"] mod optimized;

fn main() {
    let mut seed = 0x12345678u32;
    let mut cases = vec![vec![], vec![1], vec![2; 128044]];
    for len in [4, 15, 255, 1024, 4096, 65535, 128044] {
        let random: Vec<u8> = (0..len).map(|_| {
            seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
            seed as u8
        }).collect();
        cases.push(random);
        for period in [1, 2, 3, 7, 16, 255] {
            cases.push((0..len).map(|i| (i % period) as u8).collect());
        }
    }
    let mut checks = 0;
    for data in &cases {
        for level in 1..=5 {
            let old = baseline::compress_block(data, baseline::CompressionLevel::from_u32(level).unwrap()).unwrap();
            let new_level = optimized::CompressionLevel::from_u32(level).unwrap();
            let new = optimized::compress_block(data, new_level).unwrap();
            assert_eq!(old, new, "wire difference len={} level={}", data.len(), level);
            assert_eq!(optimized::decompress_block(&old, data.len()).unwrap(), *data);
            assert_eq!(baseline::decompress_block(&new, data.len()).unwrap(), *data);
            let mut prefix = [0u8; 10];
            let n = optimized::encode_uvarint(data.len(), &mut prefix);
            let prefixed = optimized::compress_block_with_prefix(data, new_level, &prefix[..n]).unwrap();
            assert_eq!(&prefixed[..n], &prefix[..n]);
            assert_eq!(&prefixed[n..], &old);
            checks += 1;
        }
    }
    // Invalid offset, truncated length and output size mismatch remain errors.
    for bad in [vec![0, 0, 0], vec![0xf0], vec![0, 1, 0]] {
        assert!(optimized::decompress_block(&bad, 20).is_err());
    }
    println!("PASS: {checks} cases, byte-identical encoding and cross-decoding");
}
