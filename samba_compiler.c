// =======================================================================================================
// ZHRXXgroup Project 🚀 - samba Build System (samba.h)
// File: samba_compiler.c
// Author(s): ZHRXXgroup
// Version: 3.0 // samba.h version: 1
// Free to use, modify, and share under our Open Source License (src.zhrxxgroup.com/OPENSOURCE_LICENSE).
// Want to contribute? Visit: issues.zhrxxgroup.com
// GitHub: https://github.com/ZHRXXgroup/samba.h
// ========================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#include "samba.h"

typedef struct {
    char** data;
    size_t size;
    size_t capacity;
} StringArray;

char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

StringArray* create_string_array(size_t initial_capacity) {
    StringArray* array = malloc(sizeof(StringArray));
    if (!array) return NULL;

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
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        char** new_data = realloc(array->data, new_capacity * sizeof(char*));
        if (!new_data) return -1;

        array->data = new_data;
        array->capacity = new_capacity;
    }

    array->data[array->size] = strdup(str);
    if (!array->data[array->size]) return -1;

    array->size++;
    return 0;
}

StringArray* parse_arguments(char* line) {
    StringArray* args = create_string_array(10);
    if (!args) return NULL;

    char* token = strtok(line, ",");
    while (token) {
        if (append_to_string_array(args, trim(token)) < 0) {
            free_string_array(args);
            return NULL;
        }
        token = strtok(NULL, ",");
    }
    return args;
}

void execute_function(const char* func_name, StringArray* args) {
    if (strcmp(func_name, "define_variable") == 0 && args->size == 2) {
        define_variable(args->data[0], args->data[1]);
    } else if (strcmp(func_name, "define_library") == 0 && args->size == 1) {
        define_library(args->data[0]);
    } else if (strcmp(func_name, "define_include") == 0 && args->size == 1) {
        define_include(args->data[0]);
    } else if (strcmp(func_name, "define_library_path") == 0 && args->size == 1) {
        define_library_path(args->data[0]);
    } else if (strcmp(func_name, "add_flag") == 0 && args->size == 1) {
        add_flag(args->data[0]);
    } else if (strcmp(func_name, "compile") == 0 && args->size == 2) {
        compile(args->data[0], args->data[1], false);
    } else if (strcmp(func_name, "enable_verbose") == 0 && args->size == 0) {
        #undef verbose_mode
        #define verbose_mode true
    } else if (strcmp(func_name, "printfn") == 0 && args->size == 1) {
        printf("%s\n", args->data[0]);
    } else if (strcmp(func_name, "check_tool") == 0 && args->size == 1) {
        check_tool(args->data[0]);
    } else if (strcmp(func_name, "reset_settings") == 0 && args->size == 0) {
        reset_settings();
    } else if (strcmp(func_name, "file_exists") == 0 && args->size == 1) {
        file_exists(args->data[0]);
    } else if (strcmp(func_name, "find_library") == 0 && args->size == 1) {
        find_library(args->data[0]);
    } else if (strcmp(func_name, "find_flags") == 0 && args->size == 1) {
        find_flags(args->data[0]);
    } else if (strcmp(func_name, "s_system") == 0 && args->size == 1) {
        s_system(args->data[0]);
    } else if (strcmp(func_name, "set_build_directory") == 0 && args->size == 1) {
        set_build_directory(args->data[0]);
    } else if (strcmp(func_name, "print_libraries") == 0 && args->size == 0) {
        print_libraries();
    } else if (strcmp(func_name, "s_system") == 0 && args->size == 1) {
        s_system(args->data[0]);
    } else if (strcmp(func_name, "clear_build_directory") == 0 && args->size == 0) {
        clear_build_directory();
    } else if (strcmp(func_name, "generate_build_report_to_file") == 0 && args->size == 1) {
        generate_build_report_to_file(args->data[0]);
    } else if (strcmp(func_name, "generate_timestamp_file") == 0 && args->size == 0) {
        generate_timestamp_file();
    } else if (strcmp(func_name, "backup_build_directory") == 0 && args->size == 1) {
        backup_build_directory(args->data[0]);
    } else if (strcmp(func_name, "check_and_install_dependency") == 0 && args->size == 1) {
        check_and_install_dependency(args->data[0]);
    } else if (strcmp(func_name, "add_memory_sanitizer") == 0 && args->size == 0) {
        add_memory_sanitizer();
    } else if (strcmp(func_name, "add_compiler_warnings") == 0 && args->size == 0) {
        add_compiler_warnings();
    } else if (strcmp(func_name, "send_notification") == 0 && args->size == 3) {
        send_notification(args->data[0], args->data[1], args->data[2]);
    } else if (strcmp(func_name, "list_defined_variables") == 0 && args->size == 0) {
        list_defined_variables();
    } else if (strcmp(func_name, "list_files_in_directory") == 0 && args->size == 1) {
        list_files_in_directory(args->data[0]);
    } else if (strcmp(func_name, "detect_compiler") == 0 && args->size == 0) {
        detect_compiler();
    } else {
        fprintf(stderr, "Unknown function or invalid arguments: %s\n", func_name);
    }
}

void parse_build_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open build file");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed_line = trim(line);
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#') continue;

        char* func_name_end = strchr(trimmed_line, '(');
        if (!func_name_end) continue;

        size_t func_name_len = func_name_end - trimmed_line;
        char* func_name = malloc(func_name_len + 1);
        if (!func_name) {
            perror("Failed to allocate memory for function name");
            continue;
        }

        strncpy(func_name, trimmed_line, func_name_len);
        func_name[func_name_len] = '\0';

        char* args_str = func_name_end + 1;
        char* args_end = strchr(args_str, ')');
        if (!args_end) {
            free(func_name);
            continue;
        }

        *args_end = '\0';
        StringArray* args = parse_arguments(args_str);
        if (!args) {
            free(func_name);
            continue;
        }

        execute_function(func_name, args);

        free(func_name);
        free_string_array(args);
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    clock_t start = clock();
    parse_build_file("build.samba");
    clock_t end = clock();

    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Build completed in %.2f seconds.\n", elapsed_time);

    return EXIT_SUCCESS;
}
