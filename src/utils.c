#include "utils.h"

void generate_random(BYTE* buffer, size_t len) {
    RtlGenRandom(buffer, len);
}

void generate_password(char* buffer, size_t len) {
    const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
    BYTE random[len];
    generate_random(random, len);
    for (size_t i = 0; i < len; i++) {
        buffer[i] = charset[random[i] % strlen(charset)];
    }
    buffer[len] = '\0';
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