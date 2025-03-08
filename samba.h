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

SCmd      smb_cmd_create();
void      smb_cmd_append(SCmd *, char *, ...);
int       smb_cmd_run_sync(SCmd *);
int       smb_cmd_run_async(SCmd *);
void      smb_cmd_reset(SCmd *);
void      smb_cmd_free(SCmd *);

char *    smb_args_shift(int *argc, char ***argv);

#endif
