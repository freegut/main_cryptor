#include "crypto.h"

// ChaCha20: минимальная реализация (на основе спецификации)
static void chacha20_block(const BYTE key[32], const BYTE nonce[12], UINT32 counter, BYTE output[64]) {
    static const UINT32 constants[4] = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
    UINT32 state[16];
    memcpy(state, constants, 16);
    memcpy(state + 4, key, 32);
    memcpy(state + 12, &counter, 4);
    memcpy(state + 13, nonce, 12);

    // 20 раундов (ChaCha20)
    for (int i = 0; i < 10; i++) {
        // Quarter Round
        #define QR(a, b, c, d) \
            a += b; d ^= a; d = (d << 16) | (d >> 16); \
            c += d; b ^= c; b = (b << 12) | (b >> 20); \
            a += b; d ^= a; d = (d << 8) | (d >> 24); \
            c += d; b ^= c; b = (b << 7) | (b >> 25);

        QR(state[0], state[4], state[8], state[12]);
        QR(state[1], state[5], state[9], state[13]);
        QR(state[2], state[6], state[10], state[14]);
        QR(state[3], state[7], state[11], state[15]);
        QR(state[0], state[5], state[10], state[15]);
        QR(state[1], state[6], state[11], state[12]);
        QR(state[2], state[7], state[8], state[13]);
        QR(state[3], state[4], state[9], state[14]);
    }

    // Добавляем начальное состояние
    for (int i = 0; i < 16; i++) state[i] += ((UINT32*)constants)[i % 4];
    memcpy(output, state, 64);
}

void chacha20_encrypt(BYTE* input, BYTE* output, size_t len, const BYTE key[32], const BYTE nonce[12]) {
    BYTE block[64];
    UINT32 counter = 0;
    for (size_t i = 0; i < len; i += 64) {
        chacha20_block(key, nonce, counter++, block);
        size_t block_len = min(64, len - i);
        memxor(input + i, block, block_len);
        memcpy(output + i, input + i, block_len);
    }
}

void chacha20_decrypt(BYTE* input, BYTE* output, size_t len, const BYTE key[32], const BYTE nonce[12]) {
    chacha20_encrypt(input, output, len, key, nonce); // ChaCha20 симметричен
}

// RSA: упрощенная реализация (только шифрование публичным ключом)
void rsa_encrypt(const BYTE* input, size_t in_len, BYTE* output, const char* pub_key) {
    // Здесь нужна полноценная RSA-реализация, но для простоты:
    // Предполагаем, что pub_key — это строка с "n" и "e" в формате "n,e"
    // Реально нужно использовать библиотеку или писать вручную (Barrett Reduction)
    memcpy(output, input, in_len); // Заглушка
}

// SHA-256: минимальная реализация
static const UINT32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, /* ... */ // Полный массив констант
};

void sha256_hash(const BYTE* input, size_t len, BYTE output[32]) {
    UINT32 h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    // TODO: Реализовать SHA-256 полностью (паддинг, циклы)
    memcpy(output, h, 32); // Заглушка
}