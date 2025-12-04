// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "main.h"
#include "../compiler/compiler.h"

#include <stdio.h>

int main(int const argc, char *argv[]) {
    if (argc < 2) {
        hello();
        return 0;
    }

    const char *filename = argv[1];
    const char result = compile(filename);
    print_result(result);
    return result;
}

void hello(void) {
    printf("Hello, this is DialScript.\n"
        "Usage: dialscript <filename>\n"
        "To use it, please refer to the documentation.\n");
}
