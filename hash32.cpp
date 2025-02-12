#include "hash32.h"

namespace Hash32 {

// хэш строки, выполняется в рантайме. Размер 32 бит
uint32_t hash(const char* str, int16_t len) {
    return _hash(str, len);
}

// хэш PROGMEM строки, выполняется в рантайме. Размер 32 бит
uint32_t hash_P(PGM_P str, int16_t len) {
    return _hash_P(str, len);
}

}  // namespace Hash32