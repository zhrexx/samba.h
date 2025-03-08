#include "../samba.h"

int main() {
    printf("My lovely Unit Tests: 😍😘\n");
    s_command("echo \"| s_command             | working ✔\"");
    if (check_tool("gcc")) printf("| check_tool            | working ✔\n");
    else printf("| check_tool            | not working ✖\n");
    if (file_exists("tests/test1")) printf("| file_exists           | working ✔\n");
    else printf("| file_exists            | not working ✖");
    if (is_internet_available()) printf("| is_internet_available | working ✔\n");
    else printf("| is_internet_available | not working ✖\n");
    if (!checkpoint_exists(999999999999)) printf("| checkpoint_exists     | working ✔\n");
    else printf("| checkpoint_exists     | not working ✖\n");
}

