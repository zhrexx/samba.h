#include "samba.h"
#include "samba_config.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define snprintf _snprintf
#else
#include <unistd.h>
#include <stdlib.h>
#endif

// ---- Macros ----
#ifdef _WIN32
#define SMB_SUDO (is_admin())

static inline bool is_admin() {
    BOOL isAdmin = FALSE;
    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
        }
        CloseHandle(token);
    }
    return isAdmin;
}
#else
#define SMB_SUDO (geteuid() == 0)
#endif

#ifdef SMBF_NO_STDBOOL
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #define bool  _Bool
#else
    typedef unsigned char bool;
#endif

#ifndef true
    #define true  1
#endif

#ifndef false
    #define false 0
#endif

#define __bool_true_false_are_defined 1
#endif

#ifdef SMBF_NO_STDARG
#if defined(__GNUC__) || defined(__clang__)
    typedef __builtin_va_list va_list;
    #define va_start(ap, last) __builtin_va_start(ap, last)
    #define va_arg(ap, type) __builtin_va_arg(ap, type)
    #define va_end(ap) __builtin_va_end(ap)
    #define va_copy(dest, src) __builtin_va_copy(dest, src)

#elif defined(_MSC_VER)
    #include <vadefs.h>
    typedef va_list va_list;
    #define va_start(ap, last) _crt_va_start(ap, last)
    #define va_arg(ap, type) _crt_va_arg(ap, type)
    #define va_end(ap) _crt_va_end(ap)
    #define va_copy(dest, src) _crt_va_copy(dest, src)
#else
    #include <stddef.h>
    typedef char* va_list;
    #define _VA_ALIGN(type) ((sizeof(type) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))
    #define va_start(ap, last) ((ap) = (char*)(&(last) + 1))
    #define va_arg(ap, type) (*(type*)(((ap) += _VA_ALIGN(type)) - _VA_ALIGN(type)))
    #define va_end(ap) ((ap) = NULL)
    #define va_copy(dest, src) ((dest) = (src))

#endif
#endif

// ------ Functions ------

SCmd smb_cmd_create() {
    SCmd cmd = {0};
    vector_init(&cmd.c, 5, sizeof(char *));
    return cmd;
}

void smb_cmd_append(SCmd *cmd, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char *arg;
    while ((arg = va_arg(args, const char *)) != NULL) {
        vector_push(&cmd->c, arg);
    }
    va_end(args);
}

int smb_cmd_run_sync(SCmd *cmd) {
    char r[256] = {0};
    for (size_t i = 0; i < vector_len(&cmd->c); i++) {
        snprintf(r + strlen(r), sizeof(r) - strlen(r), "%s", vector_get_str(&cmd->c, i));
    }

    int rt = -1;

#ifdef _WIN32
    rt = system(r);
#else
    rt = system(r);
#endif

    return rt;
}

int smb_cmd_run_async(SCmd *cmd) {
    char r[256] = {0}; 
    for (size_t i = 0; i < vector_len(&cmd->c); i++) {
        snprintf(r + strlen(r), sizeof(r) - strlen(r), "%s", vector_get_str(&cmd->c, i));
    }

#ifdef _WIN32
    int pid = _spawnl(_P_NOWAIT, "cmd.exe", "cmd.exe", "/C", r, NULL);
    if (pid == -1) {
        perror("spawn failed");
        return -1;
    }
#else
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        int status = system(r);
        _exit(status);
    }
#endif

    return 0;
}

void smb_cmd_reset(SCmd *cmd) {
    vector_free(&cmd->c);
    vector_init(&cmd->c, 5, sizeof(char *));
}

void smb_cmd_free(SCmd *cmd) {
    vector_free(&cmd->c);
}


char* smb_args_shift(int *argc, char ***argv) {
    if (*argc <= 0) return NULL;
    
    char *first_arg = (*argv)[0];
    (*argc)--;

    for (int i = 0; i < *argc; i++) {
        (*argv)[i] = (*argv)[i + 1];
    }
    (*argv)[*argc] = NULL;
    
    return first_arg;
}
