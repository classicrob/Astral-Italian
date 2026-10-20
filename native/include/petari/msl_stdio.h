#pragma once
// Narrow string formatting for legacy game code with MSL's printf semantics. Native builds rewrite the legacy sources'
// snprintf, vsnprintf, sprintf and vsprintf to these functions (native/tools/encode_legacy_sources.py), the same way
// the wide functions go to petari/utf16.h.
//
// Differences from the host C library that the game relies on (MSL printf.c):
// - a null %s argument prints nothing (the host prints "(null)"), a null %ls likewise;
// - `l` integer conversions read Wii `long`, which is 32 bits (as petari_utf16_vswprintf does);
// - an invalid conversion copies the rest of the format literally.
// The return value is the full formatted length (C99, as MSL), and the output is always terminated when n > 0.
// %ls reads a UTF-16 string (legacy wchar_t) and keeps each code unit's low byte, MSL's "C" locale conversion for the
// ASCII range. Floating-point and %p digits come from the host C library.

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int petari_msl_vsnprintf(char* dst, size_t n, const char* format, va_list args);
int petari_msl_snprintf(char* dst, size_t n, const char* format, ...);
int petari_msl_vsprintf(char* dst, const char* format, va_list args);
int petari_msl_sprintf(char* dst, const char* format, ...);

#ifdef __cplusplus
}
#endif
