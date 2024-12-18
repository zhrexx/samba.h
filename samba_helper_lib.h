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

typedef struct {
    Vector stream_out // stdout
} Stream;

Stream read_stream(char *command) {
    FILE *stream;
    char buffer[1028];

    Stream stream_data = {0};

    vector_init(&stream_data.stream_out, 10, sizeof(char *));

    stream = popen(command, "r");
    if (stream == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), stream) != NULL) {
        char *line = strdup(buffer);
        vector_push(&stream_data.stream_out, &line);
    }

    pclose(stream);

    return stream_data;

}



#endif // SAMBA_HELPER_LIB_H