// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "main.h"
#include "../compiler/compiler.h"

#include <stdio.h>
#include <string.h>

#define VERSION "0.0.1"

int main(int const argc, char *argv[]) {
    if (argc < 2) {
        hello();
        return 0;
    }

    // Default settings
    int mode = MODE_QUIET;
    const char *filename = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
            mode = MODE_VERBOSE;
        } else if (strcmp(argv[i], "--help") == 0) {
            hello();
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("\033[1;36mDialScript v%s\033[0m\n", VERSION);
            return 0;
        } else if (strcmp(argv[i], "--example") == 0) {
            example();
            return 0;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        } else {
            printf("\033[1;31mError:\033[0m unknown option '%s'\n", argv[i]);
            printf("Use 'dialscript --help' for usage information\n");
            return 1;
        }
    }

    // Check filename
    if (!filename) {
        printf("\033[1;31mError:\033[0m no input file specified\n");
        return 1;
    }

    // Check if filename ends with .ds
    const size_t len = strlen(filename);
    if (len < 3 || strcmp(filename + len - 3, ".ds") != 0) {
        printf("\033[1;31mError:\033[0m only .ds files are supported\n");
        return 1;
    }

    // Compile normally
    const char result = compile(filename, mode);
    return result;
}

// Usage info
void hello(void) {
    printf("\033[1;36mDialScript v%s\033[0m\n", VERSION);
    printf("\033[1;37mUsage:\033[0m dialscript <filename.ds> [options]\n\n");
    printf("\033[1;37mOptions:\033[0m\n");
    printf("  \033[1;32m--verbose\033[0m    Enable verbose mode\n");
    printf("  \033[1;32m--help\033[0m       Show this help message\n");
    printf("  \033[1;32m--version\033[0m    Show version number\n");
    printf("  \033[1;32m--example\033[0m    Show example .ds file\n");
}

// Example file
void example(void) {
    printf("\033[1;36mExample .ds file:\033[0m\n\n");
    printf("\033[1;36m[Scene.1]\033[0m\n");
    printf("\033[36mLevel:\033[0m 1\n");
    printf("\033[36mLocation:\033[0m Forest\n");
    printf("\033[36mCharacters:\033[0m Alan, Beth\n\n");
    printf("\033[90m// Main dialog\033[0m\n");
    printf("\033[1;35m[Dialog.1]\033[0m\n");
    printf("\033[1;37mAlan:\033[0m Hello there! \033[33m{Emotion: happy}\033[0m\n");
    printf("\033[1;37mBeth:\033[0m Hi Alan, nice to see you.\n");
    printf("\033[1;37mAlan:\033[0m Want to go for a walk?\n");
    printf("\033[1;37mBeth:\033[0m Sure! \033[33m{Choices: 1, 2}\033[0m\n");
    printf("\033[1;37mAlan:\033[0m Great, let's go! \033[33m{Choice: 1}\033[0m\n");
    printf("\033[1;37mAlan:\033[0m Maybe next time then. \033[33m{Choice: 2}\033[0m\n");
}
