// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#ifndef DIALSCRIPT_COMPILER_H
#define DIALSCRIPT_COMPILER_H

// Compiler modes
#define MODE_QUIET   0    // No verbose logs
#define MODE_VERBOSE 1    // Log each line and its evaluation

char compile(const char* filename, int verbose);    // Compile the given file
char auto_fix(const char* filename);                // Experimental auto-fix
void print_result(char result);                     // Print final result

#endif // DIALSCRIPT_COMPILER_H
