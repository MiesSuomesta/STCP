/* Host test backend: real OpenSSL X25519/AES-GCM; not the kernel/PSA backend. */
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int stcp_kernel_x25519_keypair(unsigned char *secret, unsigned char *pub)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
    EVP_PKEY *key = NULL;
    size_t n = 32;
    int ok = ctx && EVP_PKEY_keygen_init(ctx) > 0 && EVP_PKEY_keygen(ctx, &key) > 0 &&
             EVP_PKEY_get_raw_private_key(key, secret, &n) > 0;
    n = 32;
    ok = ok && EVP_PKEY_get_raw_public_key(key, pub, &n) > 0;
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(ctx);
    return ok ? 0 : -74;
}

int stcp_kernel_x25519_shared(unsigned char *out, const unsigned char *secret,
                              const unsigned char *peer)
{
    EVP_PKEY *local = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, secret, 32);
    EVP_PKEY *remote = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, peer, 32);
    EVP_PKEY_CTX *ctx = local ? EVP_PKEY_CTX_new(local, NULL) : NULL;
    size_t n = 32;
    int ok = ctx && remote && EVP_PKEY_derive_init(ctx) > 0 &&
             EVP_PKEY_derive_set_peer(ctx, remote) > 0 && EVP_PKEY_derive(ctx, out, &n) > 0;
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(local);
    EVP_PKEY_free(remote);
    return ok ? 0 : -74;
}

int stcp_kernel_derive_session_keys(const unsigned char *shared,
    const unsigned char *client, const unsigned char *server,
    unsigned char *c2s, unsigned char *s2c)
{
    static const char domain[] = "STCPv4-AES256-GCM-HKDF-SHA256";
    static const char clabel[] = "STCPv4 AES256-GCM client to server key\1";
    static const char slabel[] = "STCPv4 AES256-GCM server to client key\1";
    unsigned char input[sizeof(domain) - 1 + 64], salt[32], prk[32];
    unsigned int n;
    memcpy(input, domain, sizeof(domain)-1);
    memcpy(input+sizeof(domain)-1, client, 32);
    memcpy(input+sizeof(domain)-1+32, server, 32);
    int ok = EVP_Digest(input, sizeof(input), salt, &n, EVP_sha256(), NULL) > 0 &&
             HMAC(EVP_sha256(), salt, 32, shared, 32, prk, &n) &&
             HMAC(EVP_sha256(), prk, 32, (const unsigned char *)clabel, sizeof(clabel)-1, c2s, &n) &&
             HMAC(EVP_sha256(), prk, 32, (const unsigned char *)slabel, sizeof(slabel)-1, s2c, &n);
    OPENSSL_cleanse(prk, sizeof(prk));
    return ok ? 0 : -74;
}

static void iv_bytes(uint64_t nonce, unsigned char iv[12])
{
    memset(iv, 0, 4);
    for (int i = 0; i < 8; ++i) iv[4+i] = (unsigned char)(nonce >> (56-8*i));
}

int stcp_kernel_aes256_gcm_encrypt(const unsigned char *key, uint64_t nonce,
    const unsigned char *aad, size_t aad_len, const unsigned char *plain, size_t length,
    unsigned char *out, size_t capacity)
{
    if (capacity < length + 16) return -12;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[12]; iv_bytes(nonce, iv);
    int n = 0, written = 0;
    int ok = ctx && EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) > 0 &&
             EVP_EncryptUpdate(ctx, NULL, &n, aad, (int)aad_len) > 0 &&
             EVP_EncryptUpdate(ctx, out, &n, plain, (int)length) > 0;
    written = n;
    ok = ok && EVP_EncryptFinal_ex(ctx, out+written, &n) > 0 &&
         EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, out+length) > 0;
    EVP_CIPHER_CTX_free(ctx);
    return ok ? 0 : -74;
}

int stcp_kernel_aes256_gcm_decrypt(const unsigned char *key, uint64_t nonce,
    const unsigned char *aad, size_t aad_len, const unsigned char *cipher, size_t length,
    unsigned char *out, size_t capacity)
{
    if (length < 16 || capacity < length-16) return -74;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[12]; iv_bytes(nonce, iv);
    int n = 0, written = 0;
    int ok = ctx && EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) > 0 &&
             EVP_DecryptUpdate(ctx, NULL, &n, aad, (int)aad_len) > 0 &&
             EVP_DecryptUpdate(ctx, out, &n, cipher, (int)length-16) > 0;
    written = n;
    ok = ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *)(cipher+length-16)) > 0 &&
         EVP_DecryptFinal_ex(ctx, out+written, &n) > 0;
    EVP_CIPHER_CTX_free(ctx);
    return ok ? 0 : -74;
}

int stcp_kernel_aes256_gcm_decrypt_in_place(const unsigned char *key, uint64_t nonce,
    const unsigned char *aad, size_t aad_len, unsigned char *cipher, size_t length)
{
    return stcp_kernel_aes256_gcm_decrypt(key, nonce, aad, aad_len,
                                         cipher, length, cipher, length-16);
}

/* Host-only Noise ChaChaPoly backend. Never linked into stcp.ko. */
int stcp_p2p_noise_chacha_encrypt(const unsigned char *key, uint64_t nonce,
 const unsigned char *aad, size_t aad_len, const unsigned char *plain,
 size_t plain_len, unsigned char *out, size_t cap)
{
 unsigned char iv[12]={0};int n=0,total=0,ok;EVP_CIPHER_CTX *ctx;
 if(cap<plain_len+16 || plain_len>INT32_MAX || aad_len>INT32_MAX)return -22;
 for(int i=0;i<8;i++)iv[4+i]=(unsigned char)(nonce>>(8*i));
 ctx=EVP_CIPHER_CTX_new();
 ok=ctx && EVP_EncryptInit_ex(ctx,EVP_chacha20_poly1305(),NULL,key,iv)>0;
 if(ok && aad_len)ok=EVP_EncryptUpdate(ctx,NULL,&n,aad,(int)aad_len)>0;
 if(ok && plain_len){ok=EVP_EncryptUpdate(ctx,out,&n,plain,(int)plain_len)>0;total=n;}
 if(ok)ok=EVP_EncryptFinal_ex(ctx,out+total,&n)>0;
 if(ok)ok=EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_AEAD_GET_TAG,16,out+plain_len)>0;
 EVP_CIPHER_CTX_free(ctx);return ok?0:-74;
}
int stcp_p2p_noise_chacha_decrypt(const unsigned char *key, uint64_t nonce,
 const unsigned char *aad, size_t aad_len, const unsigned char *cipher,
 size_t cipher_len, unsigned char *out, size_t cap)
{
 unsigned char iv[12]={0};int n=0,total=0,ok;EVP_CIPHER_CTX *ctx;
 if(cipher_len<16 || cap<cipher_len-16 || cipher_len>INT32_MAX || aad_len>INT32_MAX)return -22;
 size_t len=cipher_len-16;
 for(int i=0;i<8;i++)iv[4+i]=(unsigned char)(nonce>>(8*i));
 ctx=EVP_CIPHER_CTX_new();
 ok=ctx && EVP_DecryptInit_ex(ctx,EVP_chacha20_poly1305(),NULL,key,iv)>0;
 if(ok && aad_len)ok=EVP_DecryptUpdate(ctx,NULL,&n,aad,(int)aad_len)>0;
 if(ok && len){ok=EVP_DecryptUpdate(ctx,out,&n,cipher,(int)len)>0;total=n;}
 if(ok)ok=EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_AEAD_SET_TAG,16,(void *)(cipher+len))>0;
 if(ok)ok=EVP_DecryptFinal_ex(ctx,out+total,&n)>0;
 EVP_CIPHER_CTX_free(ctx);return ok?0:-74;
}
