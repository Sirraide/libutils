#include "../include/utils.h"

extern "C" {
#include "./unicode-tables/XID_CONTINUE.c"
#include "./unicode-tables/XID_START.c"
}

LIBUTILS_NAMESPACE_BEGIN

unsigned char isstart(int c) {
    return c > MAX_XID_START ? 0 : (XID_START_TABLE[(c & ~0b111) >> 3] >> (c & 0b111)) & 1;
}
unsigned char iscontinue(int c) {
    return c > MAX_XID_CONTINUE ? 0 : (XID_CONTINUE_TABLE[(c & ~0b111) >> 3] >> (c & 0b111)) & 1;
}

LIBUTILS_NAMESPACE_END