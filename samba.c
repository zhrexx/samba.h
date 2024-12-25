#define S_AUTO
#define S_VERBOSE_MODE
#define S_REBUILD_NO_OUTPUT
#define S_CACHE_COMPILATION
#define S_RELEASE_MODE
#include "samba.h"
#include "samba_helper_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

bool check_directory() {
    if (directory_contains("./tests", "test1.c") && directory_contains(".", "samba.h")) {
        return true;
    } else {
        return false;
    }
}



int main(int argc, char *argv[]) {
    SAMBA_GO_REBUILD_URSELF();

    initialize_build_flags();

    if (CONTAINS_STRING(argv, argc, "--install")) {
	    if (S_SUDO) {
	        system("mkdir /usr/include/samba");
        	system("cp samba.h /usr/include/samba/");
        	system("cp samba_helper_lib.h /usr/include/samba/");
	    }else {
	        system("mkdir ~/.local/include");
        	system("cp samba.h ~/.local/include/samba/");
	    }
    } else if (CONTAINS_STRING(argv, argc, "--help")) {
        system("clear");
        printf("Samba Help:\n");
        printf("Author: zhrexx\n");
        printf("| --install | Installs samba.h to /usr/include/samba\n");
        printf("| --install_SP | Installs SP to /usr/bin/\n");
        printf("| --install_SC | Installs SC to /usr/bin/\n");
        printf("| --interactive | Opens a interactive menu\n");

    } else if (S_SUDO && CONTAINS_STRING(argv, argc, "--install_SP") && check_tool("gcc")) {
        compile("SambaProject.c", "SP", false);
        system("cp build/SP /usr/bin");
    } else if (CONTAINS_STRING(argv, argc, "--test")) {
        // send_notification("Test App", "Test title", "Test Message");
        char *targets[] = {"SambaProject.c", "samba_compiler.c"};
        char *outputs[] = {"SP", "SC"};

        compile_parallel(targets, outputs, 2);
    } else if (CONTAINS_STRING(argv, argc, "--interactive")) {
        interactive_menu();
    } else if (S_SUDO && CONTAINS_STRING(argv, argc, "--install_SC") && check_tool("gcc")) {
        compile("samba_compiler.c", "samba_compiler", false);
        system("cp build/samba_compiler /usr/bin/samba");
    }
    else if (CONTAINS_STRING(argv, argc, "--tests") && check_directory()) {
        s_command("gcc tests/test1.c -o tests/test1 &&clear&& ./tests/test1");
    }

    if (argc <= 1) printf("\033[0;31mUsage: samba --help\n\033[0m");

    // Plugins
    // Plugin pl = {"base", "./libbase.so"};
    // plugin_connect(&pl);

    free_all();

    return EXIT_SUCCESS;
}







