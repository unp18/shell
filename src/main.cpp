#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_builtin(const char *cmd) {
    // Add your list of builtins here
    const char *builtins[] = {"exit", "echo", "type"};
    for (int i = 0; builtins[i]; i++) {
        if (strcmp(cmd, builtins[i]) == 0)
            return 1;
    }
    return 0;
}

void type_command(const char *cmd) {
    if (is_builtin(cmd)) {
        printf("%s is a shell builtin\n", cmd);
        return;
    }

    char *path_env = getenv("PATH");
    if (!path_env) {
        printf("%s: not found\n", cmd);
        return;
    }

    // Duplicate PATH since strtok modifies it
    char *path = strdup(path_env);
    if (!path) {
        perror("strdup");
        return;
    }

    char *dir = strtok(path, ":");
    while (dir) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

        // Check if file exists and is executable
        if (access(full_path, X_OK) == 0) {
            printf("%s is %s\n", cmd, full_path);
            free(path);
            return;
        }

        dir = strtok(NULL, ":");
    }

    printf("%s: not found\n", cmd);
    free(path);
}