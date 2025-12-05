// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "compiler.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Helper functions
static char* trim(char *s) {
    while (*s && isspace(*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace(*end)) *end-- = '\0';
    return s;
}

// Helper: case-insensitive string compare
static int strcasecmp_partial(const char *s1, const char *s2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (tolower(s1[i]) != tolower(s2[i])) return 1;
    }
    return 0;
}

// Helper: check if name is a typo of a character name
static const char* find_similar_character(const char *name, const char *characters, char *correct_name, const size_t max_len) {
    if (!characters || !name);

    char chars_copy[1024];
    strncpy(chars_copy, characters, sizeof(chars_copy) - 1);
    chars_copy[sizeof(chars_copy) - 1] = '\0';

    char *token = strtok(chars_copy, ",");
    while (token) {
        // Trim whitespace
        while (*token && isspace(*token)) token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace(*end)) *end-- = '\0';

        // Check if it's a typo
        const size_t len_in = strlen(name);
        const size_t len_exp = strlen(token);

        // Too different in length
        if (len_in < len_exp - 2 || len_in > len_exp + 2) {
            token = strtok(NULL, ",");
            continue;
        }

        // Case-insensitive exact match is not a typo
        if (len_in == len_exp && strcasecmp_partial(name, token, len_in) == 0) {
            token = strtok(NULL, ",");
            continue;
        }

        // Count matching characters
        int matches = 0;
        size_t min_len = len_in < len_exp ? len_in : len_exp;
        for (size_t i = 0; i < min_len; i++) {
            if (tolower(name[i]) == tolower(token[i])) matches++;
        }

        // If more than 60% match, it's probably a typo
        const int threshold = (len_exp <= 4) ? 20 : 60;
        if ((matches * 100 / len_exp) > threshold) {
            strncpy(correct_name, token, max_len - 1);
            correct_name[max_len - 1] = '\0';
            return correct_name;
        }

        token = strtok(NULL, ",");
    }
    return NULL;
}

// Check if header word is a typo of Scene/Dialog
static const char* find_similar_header(const char *word, char *correct_word, size_t max_len) {
    const char *keywords[] = {"Scene", "Dialog", NULL};

    for (int i = 0; keywords[i]; i++) {
        const char *expected = keywords[i];
        size_t len_in = strlen(word);
        size_t len_exp = strlen(expected);

        // Too different in length
        if (len_in < len_exp - 2 || len_in > len_exp + 2) continue;

        // Case-insensitive exact match is not a typo
        if (len_in == len_exp && strcasecmp_partial(word, expected, len_in) == 0) continue;

        // Count matching characters
        int matches = 0;
        size_t min_len = len_in < len_exp ? len_in : len_exp;
        for (size_t j = 0; j < min_len; j++) {
            if (tolower(word[j]) == tolower(expected[j])) matches++;
        }

        // If more than 60% match, it's probably a typo
        if ((matches * 100 / len_exp) > 60) {
            strncpy(correct_word, expected, max_len - 1);
            correct_word[max_len - 1] = '\0';
            return correct_word;
        }
    }
    return NULL;
}

// Try to fix a line and return the fixed version
static int fix_line(const char *original, char *fixed, int max_len, const char *characters) {
    strncpy(fixed, original, max_len - 1);
    fixed[max_len - 1] = '\0';

    // Empty or comment, no fix needed
    if (original[0] == '\0' || strncmp(original, "//", 2) == 0) {
        return 0;
    }

    // Fix typos in [Scene.N] header
    if (original[0] == '[') {
        char header_word[64] = {0};
        int num;
        if (sscanf(original, "[%63[^.]", header_word) == 1 && sscanf(original, "[%*[^.]%*c%d]", &num) == 1) {
            char correct_word[64] = {0};
            const char *typo = find_similar_header(header_word, correct_word, sizeof(correct_word));
            if (typo) {
                snprintf(fixed, max_len, "[%s.%d]", correct_word, num);
                return 1;
            }
        }
        return 0;
    }

    // Fix metadata typos: Levl -> Level, Locaton -> Location, etc.
    // But skip if already correct
    if (strncmp(original, "Level:", 6) != 0 &&
        (strncasecmp(original, "Levl:", 5) == 0 ||
         strncasecmp(original, "Lvl:", 4) == 0 ||
         strncasecmp(original, "level:", 6) == 0)) {
        const char *value = strchr(original, ':');
        if (value) {
            snprintf(fixed, max_len, "Level:%s", value + 1);
            return 1;
        }
    }

    if (strncmp(original, "Location:", 9) != 0 &&
        (strncasecmp(original, "Locaton:", 8) == 0 ||
         strncasecmp(original, "Locatin:", 8) == 0 ||
         strncasecmp(original, "location:", 9) == 0)) {
        const char *value = strchr(original, ':');
        if (value) {
            snprintf(fixed, max_len, "Location:%s", value + 1);
            return 1;
        }
    }

    if (strncmp(original, "Characters:", 11) != 0 &&
        (strncasecmp(original, "Chracters:", 10) == 0 ||
         strncasecmp(original, "Characers:", 10) == 0 ||
         strncasecmp(original, "characters:", 11) == 0)) {
        const char *value = strchr(original, ':');
        if (value) {
            snprintf(fixed, max_len, "Characters:%s", value + 1);
            return 1;
        }
    }

    // Fix missing space after colon in dialog: "Alan:Hello" -> "Alan: Hello"
    char *colon = strchr(fixed, ':');
    char *meta = strchr(fixed, '{');
    if (colon && (!meta || colon < meta)) {
        if (*(colon + 1) != ' ' && *(colon + 1) != '\0') {
            const size_t pos = colon - fixed + 1;
            const size_t len = strlen(fixed);
            if (len + 1 < (size_t)max_len) {
                memmove(fixed + pos + 1, fixed + pos, len - pos + 1);
                fixed[pos] = ' ';
                return 1;
            }
        }
    }

    // Fix metadata not at end: "Alan: Hello {meta} world" -> "Alan: Hello world {meta}"
    if (meta) {
        char *closing = strchr(meta, '}');
        if (closing) {
            char *after = closing + 1;
            while (*after && isspace(*after)) after++;
            if (*after != '\0') {
                // There's text after metadata - need to move it
                char name_text[512] = {0};
                char metadata[256] = {0};
                char after_meta[256] = {0};

                // Extract parts
                size_t before_meta_len = meta - fixed;
                strncpy(name_text, fixed, before_meta_len);
                name_text[before_meta_len] = '\0';

                size_t meta_len = closing - meta + 1;
                strncpy(metadata, meta, meta_len);
                metadata[meta_len] = '\0';

                strcpy(after_meta, trim(closing + 1));

                // Reconstruct: name_text + after_meta + " " + metadata
                char *trimmed_name = trim(name_text);
                snprintf(fixed, max_len, "%s %s %s", trimmed_name, after_meta, metadata);
                return 1;
            }
        }
    }

    // Fix character name typos in dialog lines
    if (characters[0] != '\0') {
        char *colon = strchr(fixed, ':');
        if (colon && (!meta || colon < meta)) {
            char name[256] = {0};
            size_t name_len = colon - fixed;
            if (name_len < sizeof(name)) {
                strncpy(name, fixed, name_len);
                trim(name);

                // Check if this name is a typo
                char correct_name[256] = {0};
                const char *typo = find_similar_character(name, characters, correct_name, sizeof(correct_name));
                if (typo) {
                    // Replace the name
                    char rest[512] = {0};
                    strcpy(rest, colon);
                    snprintf(fixed, max_len, "%s%s", correct_name, rest);
                    return 1;
                }
            }
        }
    }

    return 0;
}

char auto_fix(const char *filename) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "tests/%s", filename);

    FILE *file = fopen(fullpath, "r");
    if (!file) {
        fprintf(stderr, "\033[1;31m✗ Error:\033[0m Cannot open file %s\n", filename);
        return 1;
    }

    char lines[1024][1024];
    char fixed_lines[1024][1024];
    int total_lines = 0;
    int fixes_count = 0;

    // Read all lines
    while (fgets(lines[total_lines], sizeof(lines[total_lines]), file)) {
        lines[total_lines][strcspn(lines[total_lines], "\n")] = 0;
        total_lines++;
    }
    fclose(file);

    printf("\033[1;36mAuto-fix:\033[0m %s\n", fullpath);

    // Fix each line
    for (int i = 0; i < total_lines; i++) {
        char characters[1024] = {0};
        for (int j = i; j >= 0; j--) {
            if (strncmp(lines[j], "Characters:", 11) == 0) {
                strncpy(characters, lines[j] + 11, sizeof(characters) - 1);
                break;
            }
        }

        if (fix_line(lines[i], fixed_lines[i], sizeof(fixed_lines[i]), characters)) {
            if (fixes_count == 0) {
                printf("\033[90m────────────────────────────────────────\033[0m\n");
            }
            printf("\033[1;34m%4d │ ◼ Fixed\033[0m\n", i + 1);
            printf("\033[90m     │   \033[31m- %s\033[0m\n", lines[i]);
            printf("\033[90m     │   \033[32m+ %s\033[0m\n", fixed_lines[i]);
            fixes_count++;
        } else {
            strcpy(fixed_lines[i], lines[i]);
        }
    }

    if (fixes_count == 0) {
        // No auto-fixes possible, check if there are errors
        char result = compile(filename, 0);
        if (result > 0) {
            printf("\033[1;31m✗ Auto-fix not possible, please fix manually\033[0m\n");
            return 1;
        }
        printf("\033[1;32m✓ No fixes needed\033[0m\n");
        return 0;
    }

    printf("\033[90m────────────────────────────────────────\033[0m\n");

    // Write fixed file
    file = fopen(fullpath, "w");
    if (!file) {
        fprintf(stderr, "\033[1;31m✗ Error:\033[0m Cannot write to file %s\n", filename);
        return 1;
    }

    for (int i = 0; i < total_lines; i++) {
        fprintf(file, "%s\n", fixed_lines[i]);
    }
    fclose(file);

    printf("\033[1;32m✓ Applied:\033[0m %d fixes\n", fixes_count);

    // Check if there are still errors after fixing
    const char result = compile(filename, 0);
    if (result > 0) {
        printf("\033[1;31m✗ Script still has errors that need to be fixed manually\033[0m\n");
        return 1;
    }

    return 0;
}
