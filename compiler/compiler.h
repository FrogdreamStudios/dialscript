// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#pragma once

// Compiler modes
#define MODE_QUIET   0    // No verbose logs
#define MODE_VERBOSE 1    // Log each line and its evaluation

char compile(const char *filename, int verbose);    // Main compile function
void print_result(char result);                     // Print final result
char auto_fix(const char *filename);                // Auto-fix function
