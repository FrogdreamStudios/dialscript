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

    int mode = MODE_QUIET;
    int fix_mode = 0;
    const char *filename = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            mode = MODE_VERBOSE;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fix") == 0) {
            fix_mode = 1;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }

    if (!filename) {
        printf("Error! No input file specified.\n");
        hello();
        return 1;
    }

    if (fix_mode) {
        return auto_fix(filename);
    }

    const char result = compile(filename, mode);
    print_result(result);
    return result;
}

void hello(void) {
    printf("Hello, this is DialScript.\n"
        "Usage: dialscript [options] <filename>\n"
        "Options:\n"
        "  -v, --verbose    Enable verbose mode\n"
        "  -f, --fix        Auto-fix errors (experimental)\n"
        "To use it, please refer to the documentation.\n");
}
