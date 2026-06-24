from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
import base64
import string
import random

STCP_AES_KEY_SIZE_IN_BYTES = 32
STCP_AES_IV_SIZE_IN_BYTES = 16

class stcpAesCodec:
    
    def generate_random_base64(self, length: int) -> bytes:
        merkit = string.ascii_letters + string.digits + string.punctuation + " "
        return ''.join(random.choice(merkit) for _ in range(length))
    
    def generate_random_base64_as_string(self, length: int) -> str:
        return self.generate_random_base64(length)

    def getPaddedKey(self, paramIn: str) -> str:

        if paramIn is None:
            return None

        padded_random = self.generate_random_base64_as_string(STCP_AES_KEY_SIZE_IN_BYTES)
        tmp = str(padded_random) + str(paramIn)
        print(f"Raw tmp : {tmp}");
        tmp = tmp[ -STCP_AES_KEY_SIZE_IN_BYTES: ]
        print(f"Clamped : {tmp}");
        return tmp[:STCP_AES_KEY_SIZE_IN_BYTES]

    def getPaddedIV(self, paramIn: str) -> str:

        if paramIn is None:
            return None

        padded_random = self.generate_random_base64_as_string(STCP_AES_IV_SIZE_IN_BYTES)
        tmp = str(padded_random) + str(paramIn)
        print(f"Raw tmp : {tmp}");
        tmp = tmp[ -STCP_AES_IV_SIZE_IN_BYTES: ]
        print(f"Clamped : {tmp}");
        return tmp[:STCP_AES_IV_SIZE_IN_BYTES]

     
    def encrypt(self, plaintext: bytes, key: str, iv: str) -> bytes:

        if plaintext is None:
            return None

        padded_key = self.getPaddedKey(key)
        padded_iv = self.getPaddedIV(key)

        # Muunna avain ja IV tavuiksi
        raw_key_bytes = padded_key.encode('utf-8') if isinstance(padded_key, str) else padded_key
        raw_iv_bytes = padded_iv.encode('utf-8') if isinstance(padded_iv, str) else padded_iv

        raw_plaintext = plaintext.encode('utf-8') if isinstance(plaintext, str) else plaintext

        # Varmista, että avain ja IV ovat oikean pituisia
        if len(raw_key_bytes) not in {16, 24, 32}:
            raise ValueError(f"Avaimen pituuden on oltava 16, 24 tai 32 tavua. Nyt {len(raw_key_bytes)}")
        if len(raw_iv_bytes) != 16:
            raise ValueError(f"IV:n pituuden on oltava 16 tavua. Nyt {len(raw_iv_bytes)}")

        # Luo AES-salausobjekti
        print(f"Len & content Key: {len(raw_key_bytes)} // {raw_key_bytes} //")
        print(f"Len & content IV:  {len(raw_iv_bytes)} // {raw_iv_bytes} //")
        cipher = Cipher(algorithms.AES(raw_key_bytes), modes.CBC(raw_iv_bytes))
        encryptor = cipher.encryptor()

        # Lisää PKCS7-padding
        padder = padding.PKCS7(algorithms.AES.block_size).padder()
        padded_plaintext = padder.update(raw_plaintext) + padder.finalize()

        # Salaa data
        return encryptor.update(padded_plaintext) + encryptor.finalize()

    def decrypt(self, ciphertext: bytes, key: str, iv: str) -> bytes:

        if ciphertext is None:
            return None

        # Muunna avain ja IV tavuiksi
        key_bytes = key.encode('utf-8') if isinstance(key, str) else key
        iv_bytes = iv.encode('utf-8') if isinstance(iv, str) else iv

        padded_key = self.getPaddedKey(key)
        padded_iv =  self.getPaddedIV(key)

        # Muunna avain ja IV tavuiksi
        raw_key_bytes = padded_key.encode('utf-8') if isinstance(padded_key, str) else padded_key
        raw_iv_bytes = padded_iv.encode('utf-8') if isinstance(padded_iv, str) else padded_iv

        # Varmista, että avain ja IV ovat oikean pituisia
        if len(raw_key_bytes) not in {16, 24, 32}:
            raise ValueError(f"Avaimen pituuden on oltava 16, 24 tai 32 tavua. Nyt {len(raw_key_bytes)}")
        if len(raw_iv_bytes) != 16:
            raise ValueError(f"IV:n pituuden on oltava 16 tavua. Nyt {len(raw_iv_bytes)}")

        # Luo AES-purkuobjekti
        print(f"Len & content Key: {len(raw_key_bytes)} // {raw_key_bytes} //")
        print(f"Len & content IV:  {len(raw_iv_bytes)} // {raw_iv_bytes} //")
        cipher = Cipher(algorithms.AES(raw_key_bytes), modes.CBC(raw_iv_bytes))
        decryptor = cipher.decryptor()
        
        print(f"Salatun datan pituus: {len(ciphertext)}")
        print(f"Salattu data: {ciphertext.hex()}") 
        
        try:
            # Purkaa data
            plaintext = decryptor.update(ciphertext) + decryptor.finalize()
            unpadder = padding.PKCS7(algorithms.AES.block_size).unpadder()
            output =  unpadder.update(plaintext) + unpadder.finalize()
            plain = output.decode()
            print(f"Plain text: len: {len(plain)} // TXT: {plain} //");
            return plain
        except ValueError as e:
            print(f"Decryption failed: {e}")
            return None
    
if __name__ == "__main__":
    # Esimerkki käytöstä
    key = b'sixteen byte key'  # 16 tavua
    iv = b'sixteen byte iv!'  # 16 tavua
    plaintext = b'This is a secret message.'

    aes_cipher = stcpAesCodec()
    ciphertext = aes_cipher.encrypt(plaintext, key, iv)
    print(f'1 Salaus: {ciphertext}')

    decrypted = aes_cipher.decrypt(ciphertext, key, iv)
    print(f'1 Purkautunut: {decrypted}')

    key = b'sixteen byte key randomiaeg 255g4g262454'  # 16 tavua
    iv = b'sixteen byte iv 24v52v54n28402!'  # 16 tavua
    plaintext = b'This is a secret message.'

    aes_cipher = stcpAesCodec()
    ciphertext = aes_cipher.encrypt(plaintext, key, iv)
    print(f'2 Salaus: {ciphertext}')

    decrypted = aes_cipher.decrypt(ciphertext, key, iv)
    print(f'2 Purkautunut: {decrypted}')
