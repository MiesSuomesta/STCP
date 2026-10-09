// Linux userspace FFI backend for STCP kernel-core AES-256-GCM.
// Kernel nonce format: 00000000 || nonce.to_be_bytes().
use aes_gcm::{Aes256Gcm, Nonce, Tag, aead::{AeadInPlace, KeyInit}};
use std::{ptr, slice};

fn iv(nonce: u64) -> [u8; 12] {
    let mut iv = [0u8; 12];
    iv[4..].copy_from_slice(&nonce.to_be_bytes());
    iv
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_aes256_gcm_encrypt(
    key: *const u8, nonce: u64, aad: *const u8, aad_len: usize,
    plain: *const u8, plain_len: usize, out: *mut u8, out_len: usize,
) -> i32 {
    if key.is_null() || out.is_null() || (aad_len != 0 && aad.is_null())
        || (plain_len != 0 && plain.is_null()) { return -libc::EINVAL; }
    let Some(required) = plain_len.checked_add(16) else { return -libc::EOVERFLOW; };
    if out_len < required { return -libc::ENOSPC; }
    let key = unsafe { slice::from_raw_parts(key, 32) };
    let aad = if aad_len == 0 { &[][..] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let plain = if plain_len == 0 { &[][..] } else { unsafe { slice::from_raw_parts(plain, plain_len) } };
    let Ok(cipher) = Aes256Gcm::new_from_slice(key) else { return -libc::EINVAL; };
    let mut data = plain.to_vec();
    let nonce_bytes = iv(nonce);
    let Ok(tag) = cipher.encrypt_in_place_detached(Nonce::from_slice(&nonce_bytes), aad, &mut data) else { return -libc::EIO; };
    unsafe {
        ptr::copy_nonoverlapping(data.as_ptr(), out, data.len());
        ptr::copy_nonoverlapping(tag.as_slice().as_ptr(), out.add(data.len()), 16);
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_aes256_gcm_decrypt(
    key: *const u8, nonce: u64, aad: *const u8, aad_len: usize,
    ciphertext: *const u8, cipher_len: usize, out: *mut u8, out_len: usize,
) -> i32 {
    if key.is_null() || ciphertext.is_null() || out.is_null()
        || (aad_len != 0 && aad.is_null()) { return -libc::EINVAL; }
    if cipher_len < 16 { return -libc::EBADMSG; }
    let plain_len = cipher_len - 16;
    if out_len < plain_len { return -libc::ENOSPC; }
    let key = unsafe { slice::from_raw_parts(key, 32) };
    let aad = if aad_len == 0 { &[][..] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let input = unsafe { slice::from_raw_parts(ciphertext, cipher_len) };
    let mut data = input[..plain_len].to_vec();
    let tag = Tag::from_slice(&input[plain_len..]);
    let Ok(cipher) = Aes256Gcm::new_from_slice(key) else { return -libc::EINVAL; };
    let nonce_bytes = iv(nonce);
    if cipher.decrypt_in_place_detached(Nonce::from_slice(&nonce_bytes), aad, &mut data, tag).is_err() {
        unsafe { ptr::write_bytes(out, 0, plain_len); }
        return -libc::EBADMSG;
    }
    unsafe { ptr::copy_nonoverlapping(data.as_ptr(), out, plain_len); }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn stcp_kernel_aes256_gcm_decrypt_in_place(
    key: *const u8, nonce: u64, aad: *const u8, aad_len: usize,
    ciphertext: *mut u8, cipher_len: usize,
) -> i32 {
    if key.is_null() || ciphertext.is_null() || (aad_len != 0 && aad.is_null()) { return -libc::EINVAL; }
    if cipher_len < 16 { return -libc::EBADMSG; }
    let plain_len = cipher_len - 16;
    let key = unsafe { slice::from_raw_parts(key, 32) };
    let aad = if aad_len == 0 { &[][..] } else { unsafe { slice::from_raw_parts(aad, aad_len) } };
    let input = unsafe { slice::from_raw_parts_mut(ciphertext, cipher_len) };
    let (body, tag_bytes) = input.split_at_mut(plain_len);
    let tag = Tag::clone_from_slice(tag_bytes);
    let Ok(cipher) = Aes256Gcm::new_from_slice(key) else { return -libc::EINVAL; };
    let nonce_bytes = iv(nonce);
    if cipher.decrypt_in_place_detached(Nonce::from_slice(&nonce_bytes), aad, body, &tag).is_err() {
        body.fill(0);
        return -libc::EBADMSG;
    }
    0
}
