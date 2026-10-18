#ifndef WCHAR_IO_H
#define WCHAR_IO_H

#include "file_struct.h"
#include "size_t.h"
#include "va_list.h"
#include "wchar_t.h"

int fwide(FILE *, int);

#ifdef __cplusplus
extern "C" {
#endif
int vswprintf(wchar_t*, size_t, const wchar_t*, va_list);
#ifdef __cplusplus
}
#endif

#endif // WCHAR_IO_H