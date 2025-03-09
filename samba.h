/*
 * SAMBA V2
 */
#ifndef SAMBA_H
#define SAMBA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "vector.h"

typedef struct {
    Vector c;
} SCmd;

void      smb_log(char *, const char *, ...);
SCmd*     smb_cmd_create();
void      smb_cmd_append(SCmd *, char *, ...);
int       smb_cmd_run_sync(SCmd *);
int       smb_cmd_run_async(SCmd *);
void      smb_cmd_reset(SCmd *);
void      smb_cmd_free(SCmd *);

char *    smb_args_shift(int *, char ***);
void      smb_rebuild_urself();
int       smb_file_exists(const char *);
int       smb_check_tool(const char *);
int       smb_check_library(const char *);
char *    smb_format(const char *, ...);
char *    smb_hnull();
#endif
