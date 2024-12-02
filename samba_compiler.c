// ! CODE IS NOT LONGER SUPPORTED PREVENT USING IT
// ! BUT IF YOUR DONT NEED FULL CAPABILITIES OF SAMBA.H LIKE FOR SMALL PROJECTS THEN YOU CAN STILL USE IT

// =======================================================================================================
// ZHRXXgroup Project 🚀 - samba Build System (samba.h)
// File: samba_compiler.c
// Author(s): ZHRXXgroup
// Version: 1 !LATEST
// Free to use, modify, and share under our Open Source License (src.zhrxxgroup.com/OPENSOURCE_LICENSE).
// Want to contribute? Visit: issues.zhrxxgroup.com
// GitHub: https://github.com/ZHRXXgroup/samba.h
// ========================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "samba.h"


char* trim(char* str) {
    while(isspace((unsigned char)*str)) str++; 
    if (*str == 0) return str; 
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--; 
    *(end + 1) = 0;
    return str;
}


char** parse_arguments(char* line, size_t* num_args) {
    size_t capacity = 10;
    char** args = malloc(capacity * sizeof(char*));
    if (!args) {
        perror("Failed to allocate memory for arguments");
        return NULL;
    }
    *num_args = 0;

    char* token = strtok(line, ",");
    while (token != NULL) {
        args[*num_args] = strdup(trim(token));
        if (!args[*num_args]) {
            perror("Failed to duplicate argument string");
            return NULL;
        }
        (*num_args)++;
        if (*num_args >= capacity) {
            capacity *= 2;
            args = realloc(args, capacity * sizeof(char*));
            if (!args) {
                perror("Failed to reallocate memory for arguments");
                return NULL;
            }
        }
        token = strtok(NULL, ",");
    }
    return args;
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
        if (!args_end) continue; 

        *args_end = '\0'; 
        size_t num_args = 0;
        char** args = parse_arguments(args_str, &num_args);
        if (!args) continue;

        
        if (strcmp(func_name, "define_variable") == 0 && num_args == 2) {
            define_variable(args[0], args[1]);
        } else if (strcmp(func_name, "define_library") == 0 && num_args == 1) {
            define_library(args[0]);
        } else if (strcmp(func_name, "define_include") == 0 && num_args == 1) {
            define_include(args[0]);
        } else if (strcmp(func_name, "define_library_path") == 0 && num_args == 1) {
            define_library_path(args[0]);
        } else if (strcmp(func_name, "add_flag") == 0 && num_args == 1) {
            add_flag(args[0]);
        } else if (strcmp(func_name, "compile") == 0 && num_args == 2) {
            compile(args[0], args[1], false);
        } else if (strcmp(func_name, "enable_verbose") == 0 && num_args == 0) {
            #undef verbose_mode
            #define verbose_mode true
        }
        else {
            fprintf(stderr, "Unknown function or invalid arguments: %s\n", func_name);
        }

        
        free(func_name);
        for (size_t i = 0; i < num_args; i++) {
            free(args[i]);
        }
        free(args);
    }

    fclose(file);
}

int main() {
    parse_build_file("build.samba");
    return 0;
}
