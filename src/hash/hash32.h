#pragma once

#include <sys/types.h>
#include <sys/pgmspace.h>

namespace Hash32 {
// хэш строки, выполняется на этапе компиляции
static constexpr uint32_t _hash_const(char const* str, const uint32_t hash = 0) {
    return (*str ? _hash_const(str + 1, hash + (hash << 5) + *str) : hash);
}

// хэш строки, выполняется в рантайме
static uint32_t _hash(const char* str, int16_t len = -1) {
    if (!str) return 0;
    uint32_t hash = 0;
    while (*str && len--) hash = hash + (hash << 5) + *str++;
    return hash;
}

// хэш PROGMEM строки, выполняется в рантайме
static uint32_t _hash_P(PGM_P str, int16_t len = -1) {
    if (!str) return 0;
    uint32_t hash = 0;
    while (len--) {
        uint8_t c = pgm_read_byte(str++);
        if (!c) break;
        hash = hash + (hash << 5) + c;
    }
    return hash;
}

// StringLength длина строки, выполняется на этапе компиляции
constexpr size_t lengthConst(char const* str, size_t len = 0) {
    return *str ? lengthConst(str + 1, len + 1) : len;
}

// StringHash хэш строки, выполняется на этапе компиляции. Размер 32 бит
constexpr uint32_t Hash(const char* str) {
    return _hash_const(str);
}


// хэш строки, выполняется в рантайме. Размер 32 бит
uint32_t hash(const char* str, int16_t len = -1);

// хэш PROGMEM строки, выполняется в рантайме. Размер 32 бит
uint32_t hash_P(PGM_P str, int16_t len = -1);

}

#if (__cplusplus >= 201103L)

constexpr uint32_t operator"" _Hash(const char* str, size_t) {
    return Hash32::_hash_const(str);
}
constexpr size_t operator"" _Length(const char*, size_t len) {
    return len;
}

constexpr uint32_t operator"" _h(const char* str, size_t) {
    return Hash32::_hash_const(str);
}
constexpr size_t operator"" _len(const char*, size_t len) {
    return len;
}
#endif