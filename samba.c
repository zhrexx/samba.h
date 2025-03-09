#include "samba.h"
#include "samba_config.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define snprintf _snprintf
#define stat _stat
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#endif
#include <stdlib.h>
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

typedef struct {
    int logging;
} SMB_State;

// ------ Variable ------
SMB_State state = {
    .logging = 1
};

//--------------------
void smb_log(char *level, const char *msg, ...) {
    if (state.logging) {
        va_list args;
        va_start(args, msg);
        printf("[%s] ", level);
        vprintf(msg, args);
        va_end(args);
        printf("\n");
    }
}

// ------ CMD ------

SCmd *smb_cmd_create() {
    SCmd *cmd = malloc(sizeof(SCmd));
    vector_init(&(cmd->c), 5, sizeof(char *));
    return cmd;
}

void smb_cmd_append(SCmd *cmd, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[1024]; 
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len < 0 || len >= (int)sizeof(buffer)) {
        perror("vsnprintf failed");
        return;
    }

    char *fmt_copy = strdup(buffer);
    if (!fmt_copy) {
        perror("strdup failed");
        return;
    }
    vector_push(&(cmd->c), &fmt_copy);

    va_start(args, fmt);
    char *arg;
    while ((arg = va_arg(args, char *)) != NULL) {
        char *arg_copy = strdup(arg);
        if (!arg_copy) {
            perror("strdup failed");
            va_end(args);
            return;
        }
        vector_push(&(cmd->c), &arg_copy);
    }
    va_end(args);
}


int smb_cmd_run_sync(SCmd *cmd) {
    size_t buffer_size = 256;
    char *r = malloc(buffer_size);
    if (!r) {
        perror("malloc failed");
        return -1;
    }
    r[0] = '\0';
    
    for (size_t i = 0; i < vector_len(&(cmd->c)); i++) {
        char *arg = vector_get_str(&(cmd->c), i);
        size_t current_len = strlen(r);
        size_t arg_len = strlen(arg);
        
        if (i > 0 && current_len > 0) {
            if (current_len + arg_len + 2 > buffer_size) {
                buffer_size *= 2;
                r = realloc(r, buffer_size);
                if (!r) {
                    perror("realloc failed");
                    return -1;
                }
            }
            strcat(r, " ");
        }
        
        if (current_len + arg_len + 1 > buffer_size) { 
            buffer_size *= 2;
            r = realloc(r, buffer_size);
            if (!r) {
                perror("realloc failed");
                return -1;
            }
        }
        strcat(r, arg);
    }
    smb_log("CMD", "%s", r);
    int rt = system(r);
    free(r);
    
    return rt;
}

int smb_cmd_run_async(SCmd *cmd) {
    size_t buffer_size = 256;
    char *r = malloc(buffer_size);
    if (!r) {
        perror("malloc failed");
        return -1;
    }
    r[0] = '\0';
    
    for (size_t i = 0; i < vector_len(&(cmd->c)); i++) {
        char *arg = vector_get_str(&(cmd->c), i);
        size_t current_len = strlen(r);
        size_t arg_len = strlen(arg);
        
        if (i > 0 && current_len > 0) {
            if (current_len + arg_len + 2 > buffer_size) {
                buffer_size *= 2;
                r = realloc(r, buffer_size);
                if (!r) {
                    perror("realloc failed");
                    return -1;
                }
            }
            strcat(r, " ");
        }
        
        if (current_len + arg_len + 1 > buffer_size) {
            buffer_size *= 2;
            r = realloc(r, buffer_size);
            if (!r) {
                perror("realloc failed");
                return -1;
            }
        }
        
        strcat(r, arg);
    }
    smb_log("CMD", "%s", r); 
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (!CreateProcess(NULL, r, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        perror("CreateProcess failed");
        free(r);
        return -1;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    free(r);
    return (int)exitCode;
#else
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        free(r);
        return -1;
    }
    
    if (pid == 0) {
        int rt = system(r);
        free(r);
        _exit(WEXITSTATUS(rt));
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    free(r);
    return WEXITSTATUS(status);
#endif
}

void smb_cmd_free(SCmd *cmd) {
    vector_free(&(cmd->c));
    free(cmd);
}

void smb_cmd_reset(SCmd *cmd) {
    smb_cmd_free(cmd);
    vector_init(&(cmd->c), 5, sizeof(char *));
}

// --------------------------------------------------------

int smb_file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
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

int smb_check_tool(const char *tool) {
    char command[256];
#ifndef _WIN32 
    snprintf(command, sizeof(command), "which %s > /dev/null 2>&1", tool);
#else 
    snprintf(command, sizeof(command), "where %s > nul 2>&1", tool);
#endif
    return (system(command) == 0);
}

int smb_check_library(const char *lib) {
    if (smb_check_tool("pkg-config")) {
        char command[256];
#ifdef _WIN32
        snprintf(command, sizeof(command), "pkg-config --exists %s", lib);
        return (system(command) == 0);
#else
        snprintf(command, sizeof(command), "pkg-config --exists %s 2>/dev/null", lib);
        if (system(command) == 0) {
            return 1;
        }
#endif
    }

#ifdef _WIN32
    char libname[256];
    snprintf(libname, sizeof(libname), "%s.dll", lib);
    HMODULE handle = LoadLibrary(libname);
    if (!handle) {
        snprintf(libname, sizeof(libname), "lib%s.dll", lib); 
        handle = LoadLibrary(libname);
    }
    if (handle) {
        FreeLibrary(handle);
        return 1;
    }
#else
    char libname[256];
    void *handle = NULL;
    
#ifdef __APPLE__
    const char *formats[] = {
        "lib%s.dylib",
        "%s.dylib",
        "lib%s.a",
        "%s.a",
        "/usr/lib/lib%s.dylib",
        "/usr/local/lib/lib%s.dylib"
    };
#else
    const char *formats[] = {
        "lib%s.so",
        "%s.so", 
        "lib%s.a",
        "%s.a",
        "/usr/lib/lib%s.so",
        "/usr/lib64/lib%s.so",
        "/usr/local/lib/lib%s.so"
    };
#endif

    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
        snprintf(libname, sizeof(libname), formats[i], lib);
        
        if (strstr(libname, ".a")) {
            FILE *f = fopen(libname, "r");
            if (f) {
                fclose(f);
                return 1;
            }
            continue;
        }
    }
    
#if defined(__linux__) && !defined(__ANDROID__)
    char ldconfig_cmd[256];
    snprintf(ldconfig_cmd, sizeof(ldconfig_cmd), 
             "ldconfig -p 2>/dev/null | grep -q \"lib%s\\.so\"", lib);
    if (system(ldconfig_cmd) == 0) {
        return 1;
    }
#endif

#endif

    return 0;
}

static int smb_needs_rebuild(const char *source_file, const char *executable) {
    struct stat source_stat, exe_stat;

    if (stat(source_file, &source_stat) != 0) {
        fprintf(stderr, "Error: Source file '%s' not found.\n", source_file);
        return 1;
    }

    if (stat(executable, &exe_stat) != 0) {
        fprintf(stderr, "Warning: Executable '%s' not found.\n", executable);
        return 1;
    }

    if (source_stat.st_mtime > exe_stat.st_mtime) {
        return 1;
    }

    return 0;
}

void smb_rebuild_urself() {
    const char *source_file = "samba.c";
    const char *executable = "samba";

    if (smb_needs_rebuild(source_file, executable) == 1) {
        SCmd *cmd = smb_cmd_create();
        smb_cmd_append(cmd, "gcc -o samba samba.c -O2 -s");
        smb_log("INFO", "Rebuilding '%s' from source '%s'.\n", executable, source_file);
        if (smb_cmd_run_async(cmd) != 0) {
            smb_log("ERROR", "Rebuild failed");
        }

        smb_log("INFO", "Build completed successfully.\n");
        system("clear");
        smb_cmd_reset(cmd);
        smb_cmd_append(cmd, "./samba");
        if (smb_cmd_run_async(cmd) != 0) {
            smb_log("ERROR", "Rerunning failed\n");
        }
    }
}


char *smb_format(const char *format, ...) {
    va_list args, args_copy;
    char *buffer = NULL;
    int length;
    if (format == NULL)
        return NULL;
    va_start(args, format);
    va_copy(args_copy, args);
    length = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (length < 0)
        return NULL;
    buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        va_end(args_copy);
        return NULL;
    }
    vsnprintf(buffer, length + 1, format, args_copy);
    va_end(args_copy);

    return buffer;
}

char *smb_hnull(void) {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    return "> NUL 2>&1";
#else
    return "> /dev/null 2>&1";
#endif
}
