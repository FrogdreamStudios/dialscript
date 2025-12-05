// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "main.h"
#include "../compiler/compiler.h"

#include <stdio.h>
#include <string.h>

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
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            mode = MODE_VERBOSE;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
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
    printf("Hello, this is DialScript.\n"
        "Usage: dialscript [options] <filename.ds>\n"
        "Options:\n"
        "  -v, --verbose    Enable verbose mode\n");
}
