#include "size_t.h"
#if defined(__MWERKS__)
__declspec(weak)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__((weak))
#endif
size_t strlen(const char* str) {
    size_t length = (size_t)-1;
    const unsigned char* p = (const unsigned char*)str - 1;
    do {
        ++length;
    } while (*++p);
    return length;
}
