// =======================================================================================================
// ZHRXXgroup Project 🚀 - samba Build System (samba.h)
// File: samba_helper_lib.h
// Author(s): ZHRXXgroup
// Version: 1.1
// Free to use, modify, and share under our Open Source License (src.zhrxxgroup.com/OPENSOURCE_LICENSE).
// Want to contribute? Visit: issues.zhrxxgroup.com
// GitHub: https://github.com/ZHRXXgroup/samba.h
// ========================================================================================================

#ifndef SAMBA_HELPER_LIB_H
#define SAMBA_HELPER_LIB_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

// #include "helper_libs/vector.h"

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

void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}

void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

char *get_formatted_time() {
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';
    return time_str;
}

double get_time_diff_ms(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) * 1000.0) +
           ((end.tv_nsec - start.tv_nsec) / 1000000.0);
}










#endif // SAMBA_HELPER_LIB_H