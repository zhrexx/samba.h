// =======================================================================================================
// ZHRXXgroup Project 🚀 - samba Build System (samba.h)
// File: samba.h
// Author(s): ZHRXXgroup
// Version: 1
// Free to use, modify, and share under our Open Source License (src.zhrxxgroup.com/OPENSOURCE_LICENSE).
// Want to contribute? Visit: issues.zhrxxgroup.com
// GitHub: https://github.com/ZHRXXgroup/samba.h
// ========================================================================================================

// -- Includes --
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
// V1.1
#include <time.h>

// INFO | Macros | Each starts with S_
// | S_AUTO | Automatic Setting of some Modes/Variables | Disabled
// | S_COMPILER | Compiler Selection                    | GCC
// | S_CACHE_COMPILATION | Sets S_COMPILER as ccache    | Disabled
// | S_VERBOSE_MODE | Setting verbose_mode to true      | Disabled
// | S_OS | Returns Compilation Target OS               | Disabled
// | S_CMP_CLANG | Used to set S_COMPILER               | Disabled
// | S_RELEASE_MODE | Setting Release Flags             | Disabled
// | S_DEBUG_MODE | Setting Debug Flags                 | Disabled
// | S_SUDO | Running as sudo?                          | NULL

// INFO | Binary Macros | Each starts with SAMBA_ and can be used inside of the compiled file
// ...

// -- Macros --
#define S_SUDO (geteuid() == 0)

#ifdef __linux__
    #define S_OS "linux"
#elif defined(__APPLE__)
    #define S_OS "macos"
#elif defined(_WIN32) || defined(_WIN64)
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


// -- Compiler --
#ifdef S_CMP_CLANG
    #ifndef S_CACHE_COMPILATION
        #define S_COMPILER "clang"
    #else
        #define S_COMPILER "ccache clang"
    #endif
#else
    #ifndef S_CACHE_COMPILATION
            #define S_COMPILER "gcc"
    #else
        #define S_COMPILER "ccache gcc"
    #endif
#endif

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

// -- Verbose Mode --
// INFO: Can be set manually
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
     struct stat info;
     return (stat(path, &info) == 0 && (info.st_mode & S_IFDIR));
}

bool check_tool(const char *tool) {
    char command[256];
    snprintf(command, sizeof(command), "which %s > /dev/null 2>&1", tool);
    return (system(command) == 0);
}

void compile(const char *script_file, const char *output_file, bool create_shared) {
    #ifdef S_CACHE_COMPILATION
        if (!check_tool("ccache")) {
            if (check_tool("dnf")) {
                system("sudo dnf install ccache -y");
                system("clear");
            }
            else if (check_tool("apt-get")) {
                system("sudo apt-get install ccache");
                system("clear");
            }
            else {
                printf("ccache not found. Please install ccache.\n");
                return;
            }
        }
    #endif
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
    }
    else {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), "-o %s/%s %s", build_directory, output_file, script_file);
        if (!build_directory_exists(build_directory)) {
            if (verbose_mode) {
            printf("Build directory '%s' does not exist. Creating it...\n", build_directory);
            }
            if (mkdir(build_directory, 0755) != 0) {
                exit_error(__func__, "Failed to create build directory");
            }
            verbose_log("Build directory created successfully.\n");
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

// INFO Returns 0 if no rebuild needed else 1
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
        verbose_log("Source file '%s' is newer than executable '%s'. Rebuild required.\n", source_file, executable);

        return 1;
    }

    verbose_log("Executable '%s' is up-to-date. Running...\n", executable);

    return 0;
}

bool file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// INFO | Idea taken from tsoding | IMPLEMENTATION SELF MADE | https://github.com/tsoding/nob.h (NOB_GO_REBUILD_URSELF)
void SAMBA_GO_REBUILD_URSELF() {
    const char *source_file = "samba.c";
    const char *executable = "samba";

    if (needs_rebuild(source_file, executable) == 1) {
        const char *build_command = "gcc -o samba samba.c -O2 -DNDEBUG -s";

        verbose_log("Rebuilding '%s' from source '%s'.\n", executable, source_file);

        if (system(build_command) != 0) {
            exit_error(__func__, "Build failed\n");
        }

        verbose_log("Build completed successfully.\n");

        system("clear");

        const char *run_command = "./samba";
        verbose_log("Executing '%s'...\n", executable);
        if (system(run_command) != 0) {
            exit_error(__func__, "Execution failed\n");
        }
    }
}

// INFO | Init for Build Modes
void initialize_build_flags() {
    #ifdef S_RELEASE_MODE
        add_flag("-O2");
        add_flag("-DNDEBUG");
        add_flag("-s");
    #endif

    #ifdef S_DEBUG_MODE
        add_flag("-O0");
        add_flag("-g");
    #endif
}

// INFO | !Uses pkgconfig
char *find_library(const char *library) {
    char command[256];
    snprintf(command, sizeof(command), "pkg-config --libs %s", library);
    FILE *pipe = popen(command, "r");
    if (!pipe) return NULL;

    char *result = malloc(128);
    fgets(result, 128, pipe);
    pclose(pipe);
    return result;
}

// INFO | !Uses pkgconfig
char *find_flags(const char *library) {
    char command[256];
    snprintf(command, sizeof(command), "pkg-config --cflags %s", library);
    FILE *pipe = popen(command, "r");
    if (!pipe) return NULL;

    char *result = malloc(128);
    fgets(result, 128, pipe);
    pclose(pipe);
    return result;
}

// INFO | Samba System Command
int s_system(char *command) {
    system(command);

    verbose_log("Executing command: %s\n", command);
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
               // V 1.1
//////////////////////////////////////////////////
//////////////////////////////////////////////////

void set_build_directory(const char *path) {
    free(build_directory);
    build_directory = strdup(path);
    if (!build_directory) {
        exit_error(__func__, "Failed to set build directory");
    }
}

void print_libraries() {
    printf("Libraries:\n");
    for (size_t i = 0; i < num_libraries; i++) {
        printf(" - %s\n", libraries[i].key);
    }
}

void clear_build_directory() {
    char command[256];

    snprintf(command, sizeof(command), "rm -rf %s", build_directory);

    verbose_log("Clearing build directory: %s\n", command);

    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to clear build directory.\n");
    }

}

void generate_build_report_to_file(const char *filename) {
    verbose_log("Generating build report to file: %s\n", filename);
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s for writing.\n", filename);
        return;
    }

    fprintf(file, "Build Configuration Report:\n");
    fprintf(file, "- Compiler: %s\n", S_COMPILER);
    fprintf(file, "- Flags: ");
    if (num_flags == 0) {
        fprintf(file, "\n  - None\n");
    } else {
        for (size_t i = 0; i < num_flags; i++) {
            fprintf(file, "\n  - %s", flags[i]);
        }
    }

    fprintf(file, "\n- Libraries: ");
    if (num_libraries == 0) {
        fprintf(file, "\n  - None\n");
    } else {
        for (size_t i = 0; i < num_libraries; i++) {
            fprintf(file, "\n  - %s", libraries[i].key);
        }
    }
    fprintf(file, "\n- Includes: ");
    if (num_includes == 0) {
        fprintf(file, "\n  - None\n");
    } else {
        for (size_t i = 0; i < num_includes; i++) {
            fprintf(file, "\n  - %s", includes[i].key);
        }
    }

    fprintf(file, "\n");

    if (fclose(file) != 0) {
        fprintf(stderr, "Error: Failed to close file %s.\n", filename);
    } else {
        printf("Build report successfully written to %s.\n", filename);
    }

    char command[256];
    snprintf(command, sizeof(command), "chmod 755 %s", filename);
    verbose_log("Executing command: %s\n", command);
    system(command);
}

void generate_timestamp_file() {
    FILE *file = fopen("build.timestamp", "w");
    if (file) {
        time_t now = time(NULL);
        fprintf(file, "Last build: %s", ctime(&now));
        fclose(file);
        printf("Build timestamp generated.\n");
    } else {
        fprintf(stderr, "Error generating timestamp file.\n");
    }
}

void backup_build_directory(const char *backup_dir) {
    char command[256];
    snprintf(command, sizeof(command), "cp -r %s %s", build_directory, backup_dir);
    verbose_log("Executing command: %s\n", command);
    system(command);
}

// -- Strip Prefix --
#ifdef S_STRIP_PREFIX
    #define AUTO S_AUTO
    #define CACHE_COMPILATION S_CACHE_COMPILATION
    #define VERBOSE_MODE S_VERBOSE_MODE
    #define OS S_OS
    #define CMP_CLANG S_CMP_CLANG
    #define RELEASE_MODE S_RELEASE_MODE
    #define DEBUG_MODE S_DEBUG_MODE
    #define SUDO S_SUDO
    #define GO_REBUILD_URSELF SAMBA_GO_REBUILD_URSELF
#endif




#endif
