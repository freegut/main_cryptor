#include <windows.h>
#include "crypto.h"
#include "utils.h"

static BYTE chacha_key[32] = {0}; // Будет заполнено билдером
static BYTE nonce[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

void decrypt_file(const char* path) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    BYTE* buffer = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
    DWORD bytesRead;
    ReadFile(hFile, buffer, fileSize.QuadPart, &bytesRead, NULL);

    chacha20_decrypt(buffer, buffer, fileSize.QuadPart, chacha_key, nonce);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, buffer, fileSize.QuadPart, &bytesRead, NULL);

    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);
}

int main(int argc, char* argv[]) {
    decrypt_file("test.txt");
    return 0;
}