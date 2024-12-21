#ifndef SAMBA_HELPER_LIB_H
#define SAMBA_HELPER_LIB_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "helper_libs/vector.h"

#ifdef S_PY // Types out python
    typedef void None;
    typedef char * str;
#endif





void printfn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}




#endif // SAMBA_HELPER_LIB_H