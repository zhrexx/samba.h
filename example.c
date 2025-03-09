#include <samba.h>

int main(int argc, char *argv[])
{
    char *c = "    ";
    if (smb_check_tool("gcc")) {
        c = "gcc";
        smb_log("INFO", "GCC found");
    } else {
        if (smb_check_tool("clang")) {
            c = "clang";
            smb_log("INFO", "CLANG found");
        } else {
            smb_log("ERROR", "No supported compiler found");
            exit(1);
        }
    }
    SCmd *cmd = smb_cmd_create();
    smb_cmd_append(cmd, c, "example.c", "-o", "example", "-O2", "-lsamba -L. -I. -static", smb_format("-Wl,-rpath=%s", getenv("PWD")), NULL);
    int r = smb_cmd_run_async(cmd);
    smb_cmd_free(cmd);

    return EXIT_SUCCESS;
}


