#ifndef NANDLOGGING_H
#define NANDLOGGING_H

#include "revolution/os.h"
#include "revolution/fs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NANDLoggingCallback)(BOOL, ISFSError);

BOOL NANDLoggingAddMessageAsync(NANDLoggingCallback, ISFSError, const char *, ...);

#ifdef __cplusplus
}
#endif

#endif // NANDLOGGING_H