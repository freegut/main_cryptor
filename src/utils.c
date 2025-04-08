#include "utils.h"
#include <wincrypt.h>

void generate_random(BYTE* buffer, size_t len) {
    HCRYPTPROV hCryptProv;
    if (CryptAcquireContextA(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hCryptProv, (DWORD)len, buffer);
        CryptReleaseContext(hCryptProv, 0);
    }
}

void generate_password(char* buffer, size_t len) {
    const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
    BYTE* random = HeapAlloc(GetProcessHeap(), 0, len);
    if (!random) return;
    generate_random(random, len);
    for (size_t i = 0; i < len; i++) {
        buffer[i] = charset[random[i] % strlen(charset)];
    }
    buffer[len] = '\0';
    HeapFree(GetProcessHeap(), 0, random);
}

void secure_zero(BYTE* buffer, size_t len) {
    SecureZeroMemory(buffer, len);
}

void write_note(const char* folder, const char* text) {
    char path[MAX_PATH];
    sprintf_s(path, sizeof(path), "%s\\README.txt", folder);
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, text, strlen(text), &written, NULL);
        CloseHandle(hFile);
    }
}