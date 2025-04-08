#ifndef CRYPTO_H
#define CRYPTO_H

#include <windows.h>

// ChaCha20
void chacha20_encrypt(BYTE* input, BYTE* output, size_t len, const BYTE key[32], const BYTE nonce[12]);
void chacha20_decrypt(BYTE* input, BYTE* output, size_t len, const BYTE key[32], const BYTE nonce[12]);

// RSA (упрощенная версия для шифрования ключа)
void rsa_encrypt(const BYTE* input, size_t in_len, BYTE* output, const char* pub_key);

// SHA-256 для проверки ключа
void sha256_hash(const BYTE* input, size_t len, BYTE output[32]);

// Вспомогательные функции
static inline void memxor(BYTE* dest, const BYTE* src, size_t len) {
    for (size_t i = 0; i < len; i++) dest[i] ^= src[i];
}

#endif