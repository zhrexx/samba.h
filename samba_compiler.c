#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#ifdef __linux__
#include <unistd.h>
#endif
#include <errno.h>

#include <samba.h>
#include "samba_config.h"

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

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

#define DEFAULT_CAPACITY 16
#define MAX_LINE_LENGTH 4096
#define MAX_PATH_LENGTH 1024

typedef struct {
    char** data;
    size_t size;
    size_t capacity;
} StringArray;

typedef struct {
    char* name;
    StringArray* dependencies;
    StringArray* commands;
    bool executed;
} Target;

typedef struct {
    Target** targets;
    size_t size;
    size_t capacity;
    char* default_target;
} BuildSystem;

static bool verbose_mode = false;
static bool quiet_mode = false;
static bool dry_run = false;
static bool color_output = true;
static char build_file[MAX_PATH_LENGTH] = "build.samba";
static BuildSystem* build_system = NULL;

void print_usage(const char* program_name);
char* trim(char* str);
char* strip(char* str, char ch);
StringArray* create_string_array(size_t initial_capacity);
void free_string_array(StringArray* array);
int append_to_string_array(StringArray* array, const char* str);
StringArray* parse_arguments(char* line);
bool execute_target(const char* target_name);
void execute_function(const char* func_name, StringArray* args);
BuildSystem* create_build_system(size_t initial_capacity);
void free_build_system(BuildSystem* system);
Target* find_target(BuildSystem* system, const char* name);
int add_target(BuildSystem* system, Target* target);
Target* create_target(const char* name);
void free_target(Target* target);
bool parse_build_file(const char* filename);
void log_message(const char* level, const char* format, ...);

char* trim(char* str) {
    if (!str) return NULL;
    
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    
    return str;
}

char* strip(char* str, char ch) {
    if (!str) return NULL;

    char* start = str;
    while (*start == ch) start++;

    char* end = str + strlen(str) - 1;
    while (end > start && *end == ch) end--;

    *(end + 1) = '\0';
    return start;
}

StringArray* create_string_array(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = DEFAULT_CAPACITY;
    
    StringArray* array = malloc(sizeof(StringArray));
    if (!array) {
        return NULL;
    }

    array->data = malloc(initial_capacity * sizeof(char*));
    if (!array->data) {
        free(array);
        return NULL;
    }

    array->size = 0;
    array->capacity = initial_capacity;
    return array;
}

void free_string_array(StringArray* array) {
    if (!array) return;
    
    for (size_t i = 0; i < array->size; i++) {
        free(array->data[i]);
    }
    
    free(array->data);
    free(array);
}

int append_to_string_array(StringArray* array, const char* str) {
    if (!array || !str) {
        return -1;
    }
    
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        char** new_data = realloc(array->data, new_capacity * sizeof(char*));
        if (!new_data) {
            return -1;
        }

        array->data = new_data;
        array->capacity = new_capacity;
    }

    array->data[array->size] = strdup(str);
    if (!array->data[array->size]) return -1;

    array->size++;
    return 0;
}

bool string_array_contains(StringArray* array, const char* str) {
    if (!array || !str) return false;
    
    for (size_t i = 0; i < array->size; i++) {
        if (strcmp(array->data[i], str) == 0) {
            return true;
        }
    }
    
    return false;
}

BuildSystem* create_build_system(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = DEFAULT_CAPACITY;
    
    BuildSystem* system = malloc(sizeof(BuildSystem));
    if (!system) {
        return NULL;
    }

    system->targets = malloc(initial_capacity * sizeof(Target*));
    if (!system->targets) {
        free(system);
        return NULL;
    }

    system->size = 0;
    system->capacity = initial_capacity;
    system->default_target = strdup("default");
    
    return system;
}

void free_build_system(BuildSystem* system) {
    if (!system) return;
    
    for (size_t i = 0; i < system->size; i++) {
        free_target(system->targets[i]);
    }
    
    free(system->targets);
    free(system->default_target);
    free(system);
}

Target* find_target(BuildSystem* system, const char* name) {
    if (!system || !name) return NULL;
    
    for (size_t i = 0; i < system->size; i++) {
        if (strcmp(system->targets[i]->name, name) == 0) {
            return system->targets[i];
        }
    }
    
    return NULL;
}

int add_target(BuildSystem* system, Target* target) {
    if (!system || !target) {
        return -1;
    }
    
    if (find_target(system, target->name)) {
        return -2;
    }
    
    if (system->size >= system->capacity) {
        size_t new_capacity = system->capacity * 2;
        Target** new_targets = realloc(system->targets, new_capacity * sizeof(Target*));
        if (!new_targets) {
            return -1;
        }

        system->targets = new_targets;
        system->capacity = new_capacity;
    }

    system->targets[system->size] = target;
    system->size++;
    
    return 0;
}

Target* create_target(const char* name) {
    if (!name) return NULL;
    
    Target* target = malloc(sizeof(Target));
    if (!target) {
        return NULL;
    }

    target->name = strdup(name);
    if (!target->name) {
        free(target);
        return NULL;
    }

    target->dependencies = create_string_array(DEFAULT_CAPACITY);
    if (!target->dependencies) {
        free(target->name);
        free(target);
        return NULL;
    }

    target->commands = create_string_array(DEFAULT_CAPACITY);
    if (!target->commands) {
        free_string_array(target->dependencies);
        free(target->name);
        free(target);
        return NULL;
    }
    
    target->executed = false;
    
    return target;
}

void free_target(Target* target) {
    if (!target) return;
    
    free(target->name);
    free_string_array(target->dependencies);
    free_string_array(target->commands);
    free(target);
}

StringArray* parse_arguments(char* line) {
    StringArray* args = create_string_array(DEFAULT_CAPACITY);
    if (!args || !line) return NULL;

    char* current = line;
    char* token_start = NULL;
    bool in_quotes = false;
    char quote_char = '\0';
    bool escaped = false;

    while (*current) {
        if (escaped) {
            escaped = false;
            current++;
            continue;
        }

        if (*current == '\\') {
            escaped = true;
            current++;
            continue;
        }

        if ((*current == '"' || *current == '\'') && (!in_quotes || *current == quote_char)) {
            if (in_quotes) {
                size_t length = current - token_start;
                char* token = malloc(length + 1);
                if (!token) {
                    free_string_array(args);
                    return NULL;
                }
                strncpy(token, token_start, length);
                token[length] = '\0';
                append_to_string_array(args, token);
                free(token);
                token_start = NULL;
                in_quotes = false;
            } else {
                in_quotes = true;
                quote_char = *current;
                token_start = current + 1;
            }
        } else if ((*current == ',' || isspace((unsigned char)*current)) && !in_quotes) {
            if (token_start) {
                size_t length = current - token_start;
                char* token = malloc(length + 1);
                if (!token) {
                    free_string_array(args);
                    return NULL;
                }
                strncpy(token, token_start, length);
                token[length] = '\0';
                
                char* trimmed_token = strdup(trim(token));
                append_to_string_array(args, trimmed_token);
                free(trimmed_token);
                free(token);
                token_start = NULL;
            }
        } else if (*current == '.' && *(current+1) == '.' && *(current+2) == '.' && !in_quotes) {
            if (token_start) {
                size_t length = current - token_start;
                char* token = malloc(length + 1);
                if (!token) {
                    free_string_array(args);
                    return NULL;
                }
                strncpy(token, token_start, length);
                token[length] = '\0';
                
                char* trimmed_token = strdup(trim(token));
                append_to_string_array(args, trimmed_token);
                free(trimmed_token);
                free(token);
            }

            append_to_string_array(args, "...");
            token_start = NULL;
            current += 3;
            continue;
        } else if (!isspace((unsigned char)*current) && !token_start) {
            token_start = current;
        }

        current++;
    }

    if (token_start) {
        size_t length = current - token_start;
        char* token = malloc(length + 1);
        if (!token) {
            free_string_array(args);
            return NULL;
        }
        strncpy(token, token_start, length);
        token[length] = '\0';
        
        char* trimmed_token = strdup(trim(token));
        append_to_string_array(args, trimmed_token);
        free(trimmed_token);
        free(token);
    }

    return args;
}

void log_message(const char* level, const char* format, ...) {
    if (quiet_mode && strcmp(level, "ERROR") != 0) {
        return;
    }
    
    char prefix[64];
    char* color_prefix = "";
    char* color_suffix = "";
    
    if (color_output) {
        if (strcmp(level, "ERROR") == 0) {
            color_prefix = COLOR_RED;
        } else if (strcmp(level, "WARNING") == 0) {
            color_prefix = COLOR_YELLOW;
        } else if (strcmp(level, "INFO") == 0) {
            color_prefix = COLOR_GREEN;
        } else if (strcmp(level, "DEBUG") == 0) {
            color_prefix = COLOR_BLUE;
        }
        color_suffix = COLOR_RESET;
    }
    
    snprintf(prefix, sizeof(prefix), "[%s%s%s] ", color_prefix, level, color_suffix);
    
    fprintf(stderr, "%s", prefix);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

bool parse_build_file(const char* filename) {
    if (!filename) {
        log_message("ERROR", "Invalid filename.");
        return false;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        log_message("ERROR", "Failed to open build file '%s': %s", filename, strerror(errno));
        exit(1);
    }
    
    build_system = create_build_system(DEFAULT_CAPACITY);
    if (!build_system) {
        log_message("ERROR", "Failed to create build system");
        fclose(file);
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    Target* current_target = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
        }
        
        char* trimmed_line = trim(line);
        
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#') {
            continue;
        }
        
        if (trimmed_line[0] != '\t' && trimmed_line[0] != ' ') {
            char* colon_pos = strchr(trimmed_line, ':');
            if (colon_pos) {
                *colon_pos = '\0';
                char* target_name = trim(trimmed_line);
                
                if (find_target(build_system, target_name)) {
                    log_message("ERROR", "Duplicate target: %s", target_name);
                    fclose(file);
                    return false;
                }
                
                current_target = create_target(target_name);
                if (!current_target) {
                    log_message("ERROR", "Failed to create target: %s", target_name);
                    fclose(file);
                    return false;
                }
                
                char* deps = trim(colon_pos + 1);
                if (strlen(deps) > 0) {
                    char* deps_copy = strdup(deps);
                    char* token = strtok(deps_copy, " \t");
                    while (token) {
                        append_to_string_array(current_target->dependencies, token);
                        token = strtok(NULL, " \t");
                    }
                    free(deps_copy);
                }
                
                add_target(build_system, current_target);
            } else {
                char* func_name_end = strchr(trimmed_line, '(');
                if (func_name_end) {
                    size_t func_name_len = func_name_end - trimmed_line;
                    char* func_name = malloc(func_name_len + 1);
                    if (!func_name) {
                        log_message("ERROR", "Failed to allocate memory for function name");
                        continue;
                    }
                    
                    strncpy(func_name, trimmed_line, func_name_len);
                    func_name[func_name_len] = '\0';
                    
                    char* args_str = func_name_end + 1;
                    char* args_end = strchr(args_str, ')');
                    if (!args_end) {
                        log_message("ERROR", "Missing closing parenthesis for function: %s", func_name);
                        free(func_name);
                        continue;
                    }
                    
                    *args_end = '\0';
                    StringArray* args = parse_arguments(args_str);
                    if (!args) {
                        log_message("ERROR", "Failed to parse arguments for function: %s", func_name);
                        free(func_name);
                        continue;
                    }
                    
                    if (strcmp(func_name, "default") == 0 && args->size == 1) {
                        free(build_system->default_target);
                        build_system->default_target = strdup(args->data[0]);
                    }
                    
                    free(func_name);
                    free_string_array(args);
                }
            }
        } else if (current_target) {
            char* command = trim(trimmed_line);
            append_to_string_array(current_target->commands, command);
        }
    }
    
    fclose(file);
    return true;
}

void execute_function(const char* func_name, StringArray* args) {
    if (!func_name || !args) {
        log_message("ERROR", "Invalid function name or arguments.");
        return;
    }
    
    if (strcmp(func_name, "smb_execute") == 0 && args->size >= 1) {
        SCmd cmd = smb_cmd_create();
        for (size_t i = 0; i < args->size; i++) {
            if (strcmp(args->data[i], "...") == 0) {
                log_message("WARNING", "Variadic arguments are expanded at this point");
                continue;
            }
            smb_cmd_append(&cmd, args->data[i]);
        }
        
        if (!dry_run) {
            if (verbose_mode) {
                log_message("INFO", "Executing: %s", args->data[0]);
            }
            smb_cmd_run_async(&cmd);
        } else {
            log_message("INFO", "Would execute: %s", args->data[0]);
        }
        
        smb_cmd_free(&cmd);
    } else {
        log_message("ERROR", "Unknown function or invalid arguments: %s", func_name);
    }
}

bool execute_target(const char* target_name) {
    if (!target_name || !build_system) {
        log_message("ERROR", "Invalid target name or build system not initialized");
        return false;
    }
    
    Target* target = find_target(build_system, target_name);
    if (!target) {
        log_message("ERROR", "Target not found: %s", target_name);
        return false;
    }
    
    if (target->executed) {
        return true;
    }
    
    for (size_t i = 0; i < target->dependencies->size; i++) {
        if (!execute_target(target->dependencies->data[i])) {
            log_message("ERROR", "Failed to execute dependency: %s", target->dependencies->data[i]);
            return false;
        }
    }
    
    if (verbose_mode) {
        log_message("INFO", "Executing target: %s", target->name);
    }
    
    for (size_t i = 0; i < target->commands->size; i++) {
        char* command = target->commands->data[i];
        
        char* func_name_end = strchr(command, '(');
        if (!func_name_end) {
            log_message("ERROR", "Invalid command format: %s", command);
            continue;
        }
        
        size_t func_name_len = func_name_end - command;
        char* func_name = malloc(func_name_len + 1);
        if (!func_name) {
            log_message("ERROR", "Failed to allocate memory for function name");
            continue;
        }
        
        strncpy(func_name, command, func_name_len);
        func_name[func_name_len] = '\0';
        
        char* args_str = func_name_end + 1;
        char* args_end = strchr(args_str, ')');
        if (!args_end) {
            log_message("ERROR", "Missing closing parenthesis for function: %s", func_name);
            free(func_name);
            continue;
        }
        
        *args_end = '\0';
        StringArray* args = parse_arguments(args_str);
        if (!args) {
            log_message("ERROR", "Failed to parse arguments for function: %s", func_name);
            free(func_name);
            continue;
        }
        
        execute_function(func_name, args);
        
        free(func_name);
        free_string_array(args);
    }
    
    target->executed = true;
    return true;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options] [targets...]\n", program_name);
    printf("Options:\n");
    printf("  -f, --file FILE      Use FILE as the build file (default: build.samba)\n");
    printf("  -v, --verbose        Enable verbose output\n");
    printf("  -q, --quiet          Suppress non-error messages\n");
    printf("  -n, --dry-run        Don't actually execute commands\n");
    printf("  -h, --help           Display this help message\n");
    printf("  --version            Display version information\n");
    printf("  --version_short      Display short version information\n");
    printf("  --no-color           Disable colored output\n");
}

int main(int argc, char* argv[]) {
    StringArray* targets_to_build = create_string_array(DEFAULT_CAPACITY);
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            free_string_array(targets_to_build);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("SambaCompiler v3\n");
            printf("| samba v%.1f\n", SMB_VERSION);
            free_string_array(targets_to_build);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "--version_short") == 0) {
            printf("v3\n");
            free_string_array(targets_to_build);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "--file") == 0 || strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                strncpy(build_file, argv[++i], MAX_PATH_LENGTH - 1);
            } else {
                log_message("ERROR", "Missing argument for %s", argv[i]);
                free_string_array(targets_to_build);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose_mode = true;
        } else if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            quiet_mode = true;
        } else if (strcmp(argv[i], "--dry-run") == 0 || strcmp(argv[i], "-n") == 0) {
            dry_run = true;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            color_output = false;
        } else if (argv[i][0] == '-') {
            log_message("ERROR", "Unknown option: %s", argv[i]);
            print_usage(argv[0]);
            free_string_array(targets_to_build);
            return EXIT_FAILURE;
        } else {
            append_to_string_array(targets_to_build, argv[i]);
        }
    }
    
    clock_t start = clock();
    
    if (!parse_build_file(build_file)) {
        log_message("ERROR", "Failed to parse build file: %s", build_file);
        free_string_array(targets_to_build);
        if (build_system) free_build_system(build_system);
        return EXIT_FAILURE;
    }
    
    if (targets_to_build->size == 0) {
        append_to_string_array(targets_to_build, build_system->default_target);
    }
    
    bool success = true;
    for (size_t i = 0; i < targets_to_build->size; i++) {
        if (!execute_target(targets_to_build->data[i])) {
            log_message("ERROR", "Failed to execute target: %s", targets_to_build->data[i]);
            success = false;
            break;
        }
    }
    
    clock_t end = clock();
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    if (elapsed_time < 0) elapsed_time = 0;
    
    if (!quiet_mode) {
        if (success) {
            log_message("INFO", "Build %scompleted in %.2f seconds.", 
                  dry_run ? "would have " : "", elapsed_time);
        } else {
            log_message("ERROR", "Build failed after %.2f seconds.", elapsed_time);
        }
    }
    
    free_string_array(targets_to_build);
    if (build_system) free_build_system(build_system);
    
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
