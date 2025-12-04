// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "compiler.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

char compile(const char *filename) {
    char fullpath[1024];
    sprintf(fullpath, "tests/%s", filename);

    // Open file
    FILE *file = fopen(fullpath, "r");
    if (!file) {
        printf("Error! Cannot open file %s\n", filename);
        return 1;
    }

    // State variables
    char line[1024];
    int line_num = 0;
    int in_scene = 0;
    int in_dialog = 0;
    int error = 0;

    // Filereader loop
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        // Remove newline
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines
        if (strlen(line) == 0) continue;

        // Skip comments
        if (strncmp(line, "//", 2) == 0) continue;

        // Check for [Scene.x]
        if (sscanf(line, "[Scene.%d]", &in_scene) == 1) {
            if (in_scene <= 0) {
                printf("Error at line %d! Invalid scene number\n", line_num);
                error = 1;
            }
            in_dialog = 0; // Reset dialog
            continue;
        }

        // If not in scene, error
        if (!in_scene) {
            printf("Error at line %d! Content outside scene\n", line_num);
            error = 1;
            continue;
        }

        // Check for Level: value
        if (strncmp(line, "Level: ", 7) == 0) {
            // Assume valid
            continue;
        }

        // Check for Location: value
        if (strncmp(line, "Location: ", 10) == 0) {
            continue;
        }

        // Check for Characters: list
        if (strncmp(line, "Characters: ", 12) == 0) {
            continue;
        }

        // Check for [Dialog.x]
        int dialog_num;
        if (sscanf(line, "[Dialog.%d]", &dialog_num) == 1) {
            if (dialog_num <= 0) {
                printf("Error at line %d! Invalid dialog number\n", line_num);
                error = 1;
            }
            in_dialog = 1;
            continue;
        }

        // If in dialog, check dialog line
        if (in_dialog) {
            char *colon = strchr(line, ':');
            if (!colon) {
                printf("Error at line %d! Invalid dialog line, missing ':'\n", line_num);
                error = 1;
                continue;
            }

            // Check if before colon is name (non-empty, no spaces?)
            *colon = 0;
            char *name = line;
            while (*name && isspace(*name)) name++;
            char *end_name = name + strlen(name) - 1;
            while (end_name > name && isspace(*end_name)) end_name--;
            *(end_name + 1) = 0;
            if (strlen(name) == 0) {
                printf("Error at line %d! Empty character name\n", line_num);
                error = 1;
                continue;
            }

            // Text after colon
            char *text = colon + 1;
            while (*text && isspace(*text)) text++;
            if (strlen(text) == 0) {
                printf("Error at line %d! Empty dialog text\n", line_num);
                error = 1;
                continue;
            }

            // Check for metadata { }
            const char *meta_start = strchr(text, '{');
            if (meta_start) {
                char *meta_end = strchr(meta_start, '}');
                if (!meta_end) {
                    printf("Error at line %d! Unclosed metadata\n", line_num);
                    error = 1;
                }
                // TODO: validate metadata content
            }
            continue;
        }

        // Unknown line
        printf("Error at line %d! Unknown syntax\n", line_num);
        error = 1;
    }

    fclose(file);
    return error;
}

void print_result(char result) {
    if (result == 0) {
        printf("\033[32mCompilation successful.\033[0m\n");
    } else {
        printf("\033[31mCompilation failed.\033[0m\n");
    }
}
