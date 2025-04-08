#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

void generate_random(BYTE* buffer, size_t len);
void generate_password(char* buffer, size_t len);
void secure_zero(BYTE* buffer, size_t len);
void write_note(const char* folder, const char* text);

#endif