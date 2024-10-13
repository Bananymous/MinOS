#pragma once
#include <stddef.h>
#include <string.h>
#include <stdint.h>
const char* strflip_internal(char* str, size_t len);
size_t itoa_internal(char* buf, size_t cap, int value);
size_t sztoa_internal(char* buf, size_t cap, size_t value);
size_t utoha_internal(char* buf, size_t cap, unsigned int value);
size_t uptrtoha_full_internal(char* buf, size_t cap, uintptr_t value);
// If end=buf we clearly had a problem parsing this
size_t atosz_internal(const char* buf, const char** end);
int atoi_internal(const char* buf, const char** end);