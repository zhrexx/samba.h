// =======================================================================================================
// ZHRXXgroup Project 🚀 - samba Build System (samba.h)
// File: samba.h
// Author(s): ZHRXXgroup
// Version: 1.1
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
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <limits.h>

// INFO | Macros | Each starts with S_
// | S_VERSION | Version of Samba                       | Samba Version
// | S_AUTO | Automatic Setting of some Modes/Variables | Disabled
// | S_COMPILER | Compiler Selection                    | GCC
// | S_CACHE_COMPILATION | Sets S_COMPILER as ccache    | Disabled
// | S_VERBOSE_MODE | Setting verbose_mode to true      | Disabled
// | S_OS | Returns Compilation Target OS               | Disabled
// | S_CMP_CLANG | Used to set S_COMPILER               | Disabled
// | S_RELEASE_MODE | Setting Release Flags             | Disabled
// | S_DEBUG_MODE | Setting Debug Flags                 | Disabled
// | S_SUDO | Running as sudo?                          | NULL
// | S_ERROR | This returns a func if its error         | -1
// | S_REBUILD_NO_OUTPUT | Displays no out on rebuild   | -1

// -- Macros --
#define S_VERSION "1.1"
#define S_SUDO (geteuid() == 0)
#define S_ERROR -1
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
#ifdef S_COMPILER
#undef S_COMPILER
#endif

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
char *checkpoints_directory = "checkpoints/";


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

/*
  @name verbose_log
  @parameters char *fmt, ...
  @description Prints an Message if verbose_mode is true
  @returns void
*/
void verbose_log(const char *fmt, ...) {
    if (verbose_mode) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

/*
  @name exit_error
  @parameters char *func, char *fmt, ...
  @description Prints an Error Message to stderr and Exits
  @returns void
*/
void exit_error(const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error at function %s: ", func);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

/*
  @name escape_argument
  @parameters char *arg
  @description Escapes an Argument for use in a shell command
  @returns char *
*/
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

/*
  @name define_variable
  @parameters char *var_name, char *var_value
  @description Adds an Variable to Built Binary
  @returns int
*/
int define_variable(const char *var_name, const char *var_value) {
    variables = realloc(variables, sizeof(Entry) * (num_variables + 1));
    if (!variables) return S_ERROR;
    variables[num_variables].key = strdup(var_name);
    if (!variables[num_variables].key) return S_ERROR;
    variables[num_variables].value = strdup(var_value);
    if (!variables[num_variables].value) return S_ERROR;
    num_variables++;
    return 0;
}

/*
  @name define_library
  @parameters char *library
  @description Adds an Library to the build configuration
  @returns int
*/
int define_library(const char *library) {
    libraries = realloc(libraries, sizeof(Entry) * (num_libraries + 1));
    if (!libraries) return S_ERROR;
    libraries[num_libraries].key = strdup(library);
    if (!libraries[num_libraries].key) return S_ERROR;
    libraries[num_libraries].value = NULL;
    num_libraries++;
    return 0;
}

/*
  @name define_include
  @parameters char *include_path
  @description Adds an Include to the build configuration
  @returns int
*/
int define_include(const char *include_path) {
    includes = realloc(includes, sizeof(Entry) * (num_includes + 1));
    if (!includes) return S_ERROR;
    includes[num_includes].key = strdup(include_path);
    if (!includes[num_includes].key) return S_ERROR;
    includes[num_includes].value = NULL;
    num_includes++;
    return 0;
}

/*
  @name define_library_path
  @parameters char *path
  @description Adds an library path to the build configuration
  @returns int
*/
int define_library_path(const char *path) {
    library_paths = realloc(library_paths, sizeof(Entry) * (num_library_paths + 1));
    if (!library_paths) return S_ERROR;
    library_paths[num_library_paths].key = strdup(path);
    if (!library_paths[num_library_paths].key) return S_ERROR;
    library_paths[num_library_paths].value = NULL;
    num_library_paths++;
    return 0;
}

/*
  @name add_flag
  @parameters char *flag
  @description Adds an flag to the build configuration
  @returns int
*/
int add_flag(const char *flag) {
    char **temp = realloc(flags, sizeof(char *) * (num_flags + 1));
    if (!temp) return S_ERROR;
    flags = temp;
    flags[num_flags] = strdup(flag);
    if (!flags[num_flags]) return S_ERROR;
    num_flags++;
    return 0;
}

/*
  @name remove_library
  @parameters char *library
  @description Removes a library from the list of libraries.
  @returns void
*/
void remove_library(const char *library) {
    for (size_t i = 0; i < num_libraries; i++) {
        if (strcmp(libraries[i].key, library) == 0) {
            free(libraries[i].key);
            libraries[i] = libraries[--num_libraries];
            return;
        }
    }
}

/*
  @name remove_include
  @parameters char *include_path
  @description Removes an include path from the list of include paths.
  @returns void
*/
void remove_include(const char *include_path) {
    for (size_t i = 0; i < num_includes; i++) {
        if (strcmp(includes[i].key, include_path) == 0) {
            free(includes[i].key);
            includes[i] = includes[--num_includes];
            return;
        }
    }
}

/*
  @name remove_library_path
  @parameters char *tool
  @description Removes a library path from the list of library paths.
  @returns void
*/
void remove_library_path(const char *path) {
    for (size_t i = 0; i < num_library_paths; i++) {
        if (strcmp(library_paths[i].key, path) == 0) {
            free(library_paths[i].key);
            library_paths[i] = library_paths[--num_library_paths];
            return;
        }
    }
}

/*
  @name build_directory_exists
  @parameters char *path
  @description PRIVATE FUNCTION
  @returns bool
*/
static bool build_directory_exists(const char *path) {
     struct stat info;
     return (stat(path, &info) == 0 && (info.st_mode & S_IFDIR));
}

/*
  @name check_tool
  @parameters char *tool
  @description Checks if a tool is installed on the system.
  @returns bool
*/
bool check_tool(const char *tool) {
    char command[256];
    snprintf(command, sizeof(command), "which %s > /dev/null 2>&1", tool);
    return (system(command) == 0);
}

/*
  @name compile
  @parameters char *script_file, char *output_file, bool create_shared
  @description Compiles a script file into an executable file with all given configuration.
  @returns void
*/
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

/*
  @name free_all
  @parameters void
  @description Frees All
  @returns void
*/
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

/*
  @name reset_settings
  @parameters void
  @description Frees All | Is made if your wanna make 2 targets or more with different flags
  @returns void
*/
void reset_settings() {
    free_all();
    #undef verbose_mode
    #define verbose_mode false
}

/*
  @name needs_rebuild
  @parameters char *source_file, char *executable
  @description Used in SAMBA_GO_REBUILD_URSELF | you can also use it
  @returns int
*/
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
        #ifndef S_REBUILD_NO_OUTPUT
            verbose_log("Source file '%s' is newer than executable '%s'. Rebuild required.\n", source_file, executable);
        #endif

        return 1;
    }

    #ifndef S_REBUILD_NO_OUTPUT
        verbose_log("Executable '%s' is up-to-date. Running...\n", executable);
    #endif
    return 0;
}

/*
  @name file_exists
  @parameters char *path
  @description Checks if the given file exists
  @returns bool
*/
bool file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/*
  @name SAMBA_GO_REBUILD_URSELF
  @parameters void
  @description Checks if the samba.c binary is uptodate | Idea taken from tsoding | Implementation self made
  @returns void
*/
void SAMBA_GO_REBUILD_URSELF() {
    const char *source_file = "samba.c";
    const char *executable = "samba";

    if (needs_rebuild(source_file, executable) == 1) {
        const char *build_command = "gcc -o samba samba.c -O2 -DNDEBUG -s";

        #ifndef S_REBUILD_NO_OUTPUT
            verbose_log("Rebuilding '%s' from source '%s'.\n", executable, source_file);
        #endif
        if (system(build_command) != 0) {
            exit_error(__func__, "Build failed\n");
        }

        #ifndef S_REBUILD_NO_OUTPUT
            verbose_log("Build completed successfully.\n");
        #endif

        system("clear");

        const char *run_command = "./samba";
        #ifndef S_REBUILD_NO_OUTPUT
            verbose_log("Executing '%s'...\n", executable);
        #endif
        if (system(run_command) != 0) {
            exit_error(__func__, "Execution failed\n");
        }
    }
}

/*
  @name initialize_build_flags
  @parameters void
  @description Inits the build flags (S_RELEASE_MODE or S_DEBUG_MODE)
  @returns void
*/
void initialize_build_flags() {
    // Modes
    #ifdef S_RELEASE_MODE
        add_flag("-O2");
        add_flag("-DNDEBUG");
        add_flag("-s");
    #endif

    #ifdef S_DEBUG_MODE
        add_flag("-O0");
        add_flag("-g");
    #endif

    // Integrate Samba Vars to the output executable
    define_variable("S_VERSION", S_VERSION);
    define_variable("S_COMPILER", S_COMPILER);
}

/*
  @name find_library
  @parameters char *library
  @description Finds libs for the given library using pkg-config
  @returns char *
*/
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

/*
  @name find_flags
  @parameters char *library
  @description Finds flags for the given library using pkg-config
  @returns char *
*/
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

/*
  @name s_command
  @parameters char *fmt, ...
  @description Simple function for executing an command (using the system function) and loging if verbose mode is enabled also supports fmt
  @returns int
*/
int s_command(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char command[256];
    vsnprintf(command, sizeof(command), fmt, args);
    verbose_log("Executing command: %s\n", command);
    va_end(args);
    return system(command);
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
               // V 1.1
//////////////////////////////////////////////////
//////////////////////////////////////////////////

/*
  @name set_build_directory
  @parameters char *path
  @description Sets the build directory to the given path
  @returns void
*/
void set_build_directory(const char *path) {
    free(build_directory);
    build_directory = strdup(path);
    if (!build_directory) {
        exit_error(__func__, "Failed to set build directory");
    }
}

/*
  @name print_libraries
  @parameters void
  @description Prints the libraries
  @returns void
*/
void print_libraries() {
    printf("Libraries:\n");
    for (size_t i = 0; i < num_libraries; i++) {
        printf(" - %s\n", libraries[i].key);
    }
}

/*
  @name create_build_directory
  @parameters void
  @description Clears the build directory
  @returns void
*/
void clear_build_directory() {
    char command[256];

    snprintf(command, sizeof(command), "rm -rf %s", build_directory);

    verbose_log("Clearing build directory: %s\n", command);

    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to clear build directory.\n");
    }

}

/*
  @name generate_build_report_to_file
  @parameters char *filename
  @description Generates a build report and writes it to a file
  @returns void
*/
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

/*
  @name generate_timestamp_file
  @parameters void
  @description Generates a timestamp file with the current date and time
  @returns void
*/
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

/*
  @name backup_build_directory
  @parameters char *backup_dir
  @description Backups the build_directory to the specified backup_dir
  @returns void
*/
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


// ! WORKS ONLY ON Red Hat based distros and on Debian based distros
/*
  @name check_and_install_dependency
  @parameters char *tool
  @description Checks if a dependency is installed and installs it if not
  @returns void
*/
void check_and_install_dependency(const char *tool) {
    if (!check_tool(tool)) {
        printf("Dependency '%s' not found. Attempting to install...\n", tool);
        if (check_tool("dnf")) {
            char command[256];
            snprintf(command, sizeof(command), "sudo dnf install -y %s", tool);
            system(command);
        } else if (check_tool("apt-get")) {
            char command[256];
            snprintf(command, sizeof(command), "sudo apt-get install -y %s", tool);
            system(command);
        } else {
            fprintf(stderr, "Error: Package manager not found. Please install '%s' manually.\n", tool);
        }
    } else {
        printf("Dependency '%s' is already installed.\n", tool);
    }
}

/*
  @name interactive_menu
  @parameters void
  @description A simple interactive menu for Samba
  @returns void
*/
void interactive_menu() {
    int choice;
    do {
        printf("\n--- Samba Menu ---\n");
        printf("1. Add Library\n");
        printf("2. Add Include Path\n");
        printf("3. Set Build Directory\n");
        printf("4. Compile\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                {
                    char lib[256];
                    printf("Enter library name: ");
                    scanf("%s", lib);
                    define_library(lib);
                }
                break;
            case 2:
                {
                    char path[256];
                    printf("Enter include path: ");
                    scanf("%s", path);
                    define_include(path);
                }
                break;
            case 3:
                {
                    char dir[256];
                    printf("Enter build directory: ");
                    scanf("%s", dir);
                    set_build_directory(dir);
                }
                break;
            case 4:
                {
                    char src[256], out[256];
                    printf("Enter source file: ");
                    scanf("%s", src);
                    printf("Enter output file: ");
                    scanf("%s", out);
                    compile(src, out, false);
                }
                break;
            case 5:
                printf("Exiting menu.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 5);
}

/*
  @name add_memory_sanitizer
  @parameters void
  @description Adds memory sanitizer flags to the flags
  @returns void
*/
void add_memory_sanitizer() {
    add_flag("-fsanitize=address");
    add_flag("-fsanitize=undefined");
}

/*
  @name add_compiler_warnings
  @parameters void
  @description Adds compiler warnings to the flags
  @returns void
*/
void add_compiler_warnings() {
    add_flag("-Wall");
    add_flag("-Wextra");
    add_flag("-pedantic");
}

/*
  @name send_notification
  @parameters char *app_name, char *title, char *message
  @description Sends a notification using the notify-send command
  @returns void
*/
void send_notification(const char *app_name, const char *title, const char *message) {
    char command[512];
    snprintf(command, sizeof(command), "notify-send '%s' '%s' -a '%s'", title, message, app_name);
    system(command);
}

/*
  @name list_defined_variables
  @parameters void
  @description Prints all defined vars
  @returns void
*/
void list_defined_variables() {
    printf("Defined Variables:\n");
    for (size_t i = 0; i < num_variables; i++) {
        printf(" - %s = %s\n", variables[i].key, variables[i].value);
    }
}

/*
  @name get_current_datetime
  @parameters char *buffer, size_t buffer_size
  @description Get current datetime and store it in the buffer
  @returns void
*/
void get_current_datetime(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

/*
  @name is_internet_available
  @parameters void
  @description Checks if internet is available
  @returns bool
*/
bool is_internet_available() {
    char buffer[256];
    FILE *fp = popen("ping -c 1 8.8.8.8 > /dev/null 2>&1", "r");
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    return true;
}


/*
  @name list_files_in_directory
  @parameters char *dir_path
  @description Print all files in a directory
  @returns void
*/
void list_files_in_directory(const char *dir_path) {
    struct dirent *entry;
    DIR *dp = opendir(dir_path);
    if (dp == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dp);
}


/*
  @name get_process_id
  @parameters void
  @description Get the process ID of the current process
  @returns pid_t
*/
pid_t get_process_id() {
    return getpid();
}


/*
  @name check_library
  @parameters char *library
  @description Checks if a library is installed using pkg-config
  @returns bool
*/
bool check_library(const char *library) {
    char command[256];
    snprintf(command, sizeof(command), "pkg-config --exists %s > /dev/null 2>&1", library);
    return (system(command) == 0);
}

/*
  @name add_no_debug
  @parameters void
  @description Sets the NDEBUG flag
  @returns void
*/
void add_no_debug() {
    add_flag("-DNDEBUG");
}

typedef struct {
    char *target;
    char *output;
    bool create_shared;
} compile_args_t;

void *compile_wrapper(void *args) {
    compile_args_t *compile_args = (compile_args_t *)args;
    compile(compile_args->target, compile_args->output, compile_args->create_shared);
    return NULL;
}

/*
  @name compile_parallel
  @parameters char **targets, char **outputs, int num_targets
  @description Compiles the targets in parallel (instead of define threads define num_targets)
  @returns int
*/
int compile_parallel(char **targets, char **outputs, int num_targets) {
    pthread_t thread_ids[num_targets];
    for (int i = 0; i < num_targets; i++) {
        compile_args_t *args = (compile_args_t *)malloc(sizeof(compile_args_t));
        if (args == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return S_ERROR;
        }

        args->target = targets[i];
        args->output = outputs[i];
        args->create_shared = false;

        if (pthread_create(&thread_ids[i], NULL, compile_wrapper, (void *)args) != 0) {
            fprintf(stderr, "Error: Unable to create thread\n");
            free(args);
            return S_ERROR;
        }
    }

    for (int i = 0; i < num_targets; i++) {
        pthread_join(thread_ids[i], NULL);
    }


}

long get_biggest_number_in_dir(const char* directory_path) {
    DIR *dir;
    struct dirent *entry;
    long biggest_num = 0;
    char *endptr;
    char full_path[PATH_MAX];
    struct stat path_stat;

    dir = opendir(directory_path);
    if (dir == NULL) {
        mkdir(directory_path, 0777);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);

        if (stat(full_path, &path_stat) != 0) {
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            long current_num = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0' && entry->d_name[0] != '\0') {
                if (current_num > biggest_num) {
                    biggest_num = current_num;
                }
            }
        }
    }

    closedir(dir);
    return biggest_num;
}

/*
  @name checkpoint_backup
  @parameters void
  @description Copies compiled binaries to a checkpoint directory
  @returns void
*/
void checkpoint_backup() {
    char dir_name[32];
    long next_num;

    next_num = get_biggest_number_in_dir(checkpoints_directory) + 1;

    snprintf(dir_name, sizeof(dir_name), "%ld", next_num);

    if (dir_name[0] != '\0') {
        s_command("mkdir -p %s%s", checkpoints_directory, dir_name);
        s_command("cp -r %s/* %s%s/", build_directory, checkpoints_directory, dir_name);
    }
}

void restore_checkpoint(long checkpoint_num) {
    char dir_name[32];
    snprintf(dir_name, sizeof(dir_name), "%ld", checkpoint_num);

    if (access(build_directory, F_OK) != 0) {
        s_command("mkdir -p %s", build_directory);
    }

    s_command("cp -r %s%s/* %s/", checkpoints_directory, dir_name, build_directory);
}

void list_checkpoints() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(checkpoints_directory);
    if (dir == NULL) {
        fprintf(stderr, "Error: Could not open checkpoints directory\n");
        return;
    }

    printf("Available checkpoints:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
            printf("Checkpoint %s\n", entry->d_name);
        }
    }
    closedir(dir);
}

void delete_checkpoint(long checkpoint_num) {
    char dir_name[32];
    snprintf(dir_name, sizeof(dir_name), "%ld", checkpoint_num);
    s_command("rm -rf %s%s", checkpoints_directory, dir_name);
}

#endif



#endif


