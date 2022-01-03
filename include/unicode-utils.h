#ifndef UTILS_UNICODE_UTILS_H
#define UTILS_UNICODE_UTILS_H

#include "./utils.h"

#define LIBUTILS_MAX_XID_CONTINUE (0x3134A)
#define LIBUTILS_MAX_XID_START    (0x3134A)

LIBUTILS_NAMESPACE_BEGIN

unsigned char isstart(int c);
unsigned char iscontinue(int c);

LIBUTILS_NAMESPACE_END

#endif // UTILS_UNICODE_UTILS_H
