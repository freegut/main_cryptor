#include "config.h"
#include <string.h>

Config parse_config(const char* json) {
    Config config = {0};
    config.stripe_percent[0] = 2.0f; // до 100 МБ
    config.stripe_percent[1] = 1.0f; // 100 МБ - 1 ГБ
    config.stripe_percent[2] = 0.6f; // 1 ГБ - 100 ГБ
    config.stripe_percent[3] = 0.6f; // 100 ГБ - 1 ТБ
    config.stripe_percent[4] = 0.6f; // > 1 ТБ

    // Минимальный парсер JSON (ищем ключ-значение)
    if (strstr(json, "\"rsa_pub_key\"")) {
        config.rsa_pub_key = strdup(strstr(json, "\"rsa_pub_key\"") + 14);
    }
    if (strstr(json, "\"note_text\"")) {
        config.note_text = strdup(strstr(json, "\"note_text\"") + 12);
    }
    if (strstr(json, "\"log_enabled\": true")) {
        config.log_enabled = TRUE;
    }

    return config;
}

void free_config(Config* config) {
    if (config->rsa_pub_key) free(config->rsa_pub_key);
    if (config->note_text) free(config->note_text);
    if (config->target_path) free(config->target_path);
}