#include "shell.h"
#include "../vga/vga.h"
#include "../keyboard/keyboard.h"
#include "../fs/vfs.h"
#include "../string/string.h"
#include "../process/task.h"

void init_shell() {
    while (1) {
        vga_print("BOS> ");
        char input[100];
        get_string(input, 100);

        if (strcmp(input, "ls") == 0) {
            int i = 0;
            struct dirent *node = 0;
            while ((node = readdir_fs(fs_root, i)) != 0) {
                vga_print(node->name);
                vga_print("\n");
                i++;
            }
        } else if (strcmp(input, "testfork") == 0) {
            int pid = fork();
            if (pid == 0) {
                vga_print("I am the child, and I will now exit.\n");
                exit();
            } else {
                vga_print("I am the parent, child pid is ");
                char pid_str[10];
                itoa(pid, pid_str, 10);
                vga_print(pid_str);
                vga_print("\n");
            }
        } else if (strcmp(input, "pid") == 0) {
            char pid_str[10];
            itoa(getpid(), pid_str, 10);
            vga_print("Current PID: ");
            vga_print(pid_str);
            vga_print("\n");
        } else if (strlen(input) > 0) {
            vga_print("Unknown command: ");
            vga_print(input);
            vga_print("\n");
        }
    }
}
