// !! USE ONLY AT WINDOWS !!
// !! ONLY UP TO 1.1 !!
// !! EXPERIMENTAL ONLY !!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <direct.h>
#include <stdarg.h>
#include <time.h>

#define S_SUDO (IsUserAnAdmin())

#ifdef _WIN32
    #define S_OS "windows"
#else
    #define S_OS "unknown"
#endif

#define CONTAINS_STRING(array, size, target) ({         \
    int found = 0;                                      \
    for (int i = 0; i < (size); i++) {                  \
        if (strcmp((array)[i], (target)) == 0) {        \
            found = 1;                                  \
            break;                                      \
        }                                               \
    }                                                   \
    found;                                              \
})

#define S_COMPILER "gcc"

char *build_directory = "build";

#ifndef SAMBA_H
#define SAMBA_H

typedef struct {
    char *key;
    char *value;
} Entry;

Entry *libraries = NULL;
Entry *includes = NULL;
Entry *library_paths = NULL;
Entry *variables = NULL;
char **flags = NULL;
size_t num_libraries = 0;
size_t num_includes = 0;
size_t num_library_paths = 0;
size_t num_variables = 0;
size_t num_flags = 0;

#ifdef S_VERBOSE_MODE
    #define verbose_mode true
#else
    #define verbose_mode false
#endif

void verbose_log(const char *fmt, ...) {
    if (verbose_mode) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

void exit_error(const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error at function %s: ", func);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

char *escape_argument(const char *arg) {
    size_t len = strlen(arg);
    char *escaped = malloc(len * 2 + 3);
    if (!escaped) return NULL;
    char *out = escaped;
    *out++ = '"';
    for (const char *in = arg; *in; in++) {
        if (*in == '"' || *in == '\\') *out++ = '\\';
        *out++ = *in;
    }
    *out++ = '"';
    *out = '\0';
    return escaped;
}

int define_variable(const char *var_name, const char *var_value) {
    variables = realloc(variables, sizeof(Entry) * (num_variables + 1));
    if (!variables) return -1;
    variables[num_variables].key = strdup(var_name);
    if (!variables[num_variables].key) return -1;
    variables[num_variables].value = strdup(var_value);
    if (!variables[num_variables].value) return -1;
    num_variables++;
    return 0;
}

int define_library(const char *library) {
    libraries = realloc(libraries, sizeof(Entry) * (num_libraries + 1));
    if (!libraries) return -1;
    libraries[num_libraries].key = strdup(library);
    if (!libraries[num_libraries].key) return -1;
    libraries[num_libraries].value = NULL;
    num_libraries++;
    return 0;
}

int define_include(const char *include_path) {
    includes = realloc(includes, sizeof(Entry) * (num_includes + 1));
    if (!includes) return -1;
    includes[num_includes].key = strdup(include_path);
    if (!includes[num_includes].key) return -1;
    includes[num_includes].value = NULL;
    num_includes++;
    return 0;
}

int define_library_path(const char *path) {
    library_paths = realloc(library_paths, sizeof(Entry) * (num_library_paths + 1));
    if (!library_paths) return -1;
    library_paths[num_library_paths].key = strdup(path);
    if (!library_paths[num_library_paths].key) return -1;
    library_paths[num_library_paths].value = NULL;
    num_library_paths++;
    return 0;
}

int add_flag(const char *flag) {
    char **temp = realloc(flags, sizeof(char *) * (num_flags + 1));
    if (!temp) return -1;
    flags = temp;
    flags[num_flags] = strdup(flag);
    if (!flags[num_flags]) return -1;
    num_flags++;
    return 0;
}

void remove_library(const char *library) {
    for (size_t i = 0; i < num_libraries; i++) {
        if (strcmp(libraries[i].key, library) == 0) {
            free(libraries[i].key);
            libraries[i] = libraries[--num_libraries];
            return;
        }
    }
}

void remove_include(const char *include_path) {
    for (size_t i = 0; i < num_includes; i++) {
        if (strcmp(includes[i].key, include_path) == 0) {
            free(includes[i].key);
            includes[i] = includes[--num_includes];
            return;
        }
    }
}

void remove_library_path(const char *path) {
    for (size_t i = 0; i < num_library_paths; i++) {
        if (strcmp(library_paths[i].key, path) == 0) {
            free(library_paths[i].key);
            library_paths[i] = library_paths[--num_library_paths];
            return;
        }
    }
}

static bool build_directory_exists(const char *path) {
    DWORD ftyp = GetFileAttributes(path);
    return (ftyp != INVALID_FILE_ATTRIBUTES && (ftyp & FILE_ATTRIBUTE_DIRECTORY));
}

bool check_tool(const char *tool) {
    char command[256];
    snprintf(command, sizeof(command), "where %s > NUL 2>&1", tool);
    return (system(command) == 0);
}

void compile(const char *script_file, const char *output_file, bool create_shared) {
    char command[4096];
    snprintf(command, sizeof(command), "%s ", S_COMPILER);

    for (size_t i = 0; i < num_variables; i++) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-D%s='\"%s\"' ", variables[i].key, variables[i].value);
    }
    for (size_t i = 0; i < num_includes; i++) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-I%s ", includes[i].key);
    }
    for (size_t i = 0; i < num_library_paths; i++) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-L%s ", library_paths[i].key);
    }
    for (size_t i = 0; i < num_libraries; i++) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-l%s ", libraries[i].key);
    }
    for (size_t i = 0; i < num_flags; i++) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "%s ", flags[i]);
    }
    if (create_shared) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-shared ");
    }
    if (build_directory == NULL) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-o %s %s", output_file, script_file);
    } else {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-o %s\\%s %s", build_directory, output_file, script_file);
        if (!build_directory_exists(build_directory)) {
            if (mkdir(build_directory) != 0) {
                exit_error(__func__, "Failed to create build directory");
            }
        }
    }

    verbose_log("Executing command: %s\n", command);
    if (system(command) != 0) {
        fprintf(stderr, "Error: Compilation failed.\n");
    } else {
        printf("Compilation successful: %s\n", output_file);
    }
}

void free_all() {
    for (size_t i = 0; i < num_libraries; i++) free(libraries[i].key);
    free(libraries);
    for (size_t i = 0; i < num_includes; i++) free(includes[i].key);
    free(includes);
    for (size_t i = 0; i < num_library_paths; i++) free(library_paths[i].key);
    free(library_paths);
    for (size_t i = 0; i < num_flags; i++) free(flags[i]);
    free(flags);
    libraries = includes = library_paths = NULL;
    flags = NULL;
    num_libraries = num_includes = num_library_paths = num_flags = 0;
}

void reset_settings() {
    free_all();
    #undef verbose_mode
    #define verbose_mode false
}

int needs_rebuild(const char *source_file, const char *executable) {
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
        printf("Source file is newer. Rebuilding...\n");
        return 1;
    }

    return 0;
}