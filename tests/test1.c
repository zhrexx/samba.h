#include "../samba.h"

int main() {
    s_command("echo \"s_command             | working\"");
    if (check_tool("gcc")) printf("check_tool            | working\n");
    if (file_exists("tests/test1")) printf("file_exists           | working\n");
    else printf("file_exists            | not working");
    if (is_internet_available()) printf("is_internet_available | working\n");
    if (!checkpoint_exists(999999999999)) printf("checkpoint_exists     | working\n");

}

