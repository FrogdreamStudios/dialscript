// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "compiler.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Helper functions
static char *trim(char *s) {
    while (*s && isspace(*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace(*end)) *end-- = '\0';
    return s;
}

// Levenshtein distance function
// Source: https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
static int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            const int cost = (tolower(s1[i - 1]) == tolower(s2[j - 1])) ? 0 : 1;
            matrix[i][j] = (matrix[i - 1][j] + 1 < matrix[i][j - 1] + 1) ? matrix[i - 1][j] + 1 : matrix[i][j - 1] + 1;
            if (matrix[i][j] > matrix[i - 1][j - 1] + cost)
                matrix[i][j] = matrix[i - 1][j - 1] + cost;
        }
    }
    return matrix[len1][len2];
}

// Generalized function to find similar string from candidates
static const char *find_similar(const char *input, const char **candidates) {
    for (int i = 0; candidates[i]; i++) {
        const char *cand = candidates[i];
        const int dist = levenshtein_distance(input, cand);
        const int len_in = strlen(input), len_cand = strlen(cand);
        if (dist <= 1 && !(len_in == len_cand && strcasecmp(input, cand) == 0)) {
            return cand;
        }
    }
    return NULL;
}

static int fix_header_typos(char *fixed, int max_len) {
    if (fixed[0] != '[') return 0;
    char header_word[64];
    int num;
    if (sscanf(fixed, "[%63[^.]", header_word) != 1 || sscanf(fixed, "[%*[^.]%*c%d]", &num) != 1) return 0;
    const char *keywords[] = {"Scene", "Dialog", NULL};
    const char *correct = find_similar(header_word, keywords);
    if (correct) {
        snprintf(fixed, max_len, "[%s.%d]", correct, num);
        return 1;
    }
    return 0;
}

static int fix_metadata_typos(char *fixed, int max_len) {
    static const struct {
        const char *wrong, *right;
    } fixes[] = {
                {"Levl:", "Level:"}, {"Lvl:", "Level:"}, {"level:", "Level:"},
                {"Locaton:", "Location:"}, {"Locatin:", "Location:"}, {"location:", "Location:"},
                {"Chracters:", "Characters:"}, {"Characers:", "Characters:"}, {"characters:", "Characters:"},
                {NULL, NULL}
            };
    for (int i = 0; fixes[i].wrong; i++) {
        size_t len = strlen(fixes[i].wrong);
        if (strncasecmp(fixed, fixes[i].wrong, len) == 0 && fixed[len] == ':') {
            char *value = fixed + len;
            memmove(fixed, fixes[i].right, strlen(fixes[i].right));
            memmove(fixed + strlen(fixes[i].right), value, strlen(value) + 1);
            return 1;
        }
    }
    return 0;
}

static int fix_missing_space_after_colon(char *fixed, int max_len) {
    char *colon = strchr(fixed, ':');
    char *meta = strchr(fixed, '{');
    if (!colon || (meta && colon > meta)) return 0;
    if (colon[1] != ' ' && colon[1] != '\0') {
        size_t pos = colon - fixed + 1;
        size_t len = strlen(fixed);
        if (len + 1 >= (size_t) max_len) return 0;
        memmove(fixed + pos + 1, fixed + pos, len - pos + 1);
        fixed[pos] = ' ';
        return 1;
    }
    return 0;
}

static int fix_metadata_position(char *fixed, int max_len) {
    char *meta = strchr(fixed, '{');
    if (!meta) return 0;
    char *closing = strchr(meta, '}');
    if (!closing) return 0;
    char *after = closing + 1;
    while (*after && isspace(*after)) after++;
    if (*after == '\0') return 0;
    char name_text[512];
    size_t before_len = meta - fixed;
    strncpy(name_text, fixed, before_len);
    name_text[before_len] = '\0';
    char metadata[256];
    size_t meta_len = closing - meta + 1;
    strncpy(metadata, meta, meta_len);
    metadata[meta_len] = '\0';
    char after_meta[256];
    strcpy(after_meta, trim(closing + 1));
    char *trimmed_name = trim(name_text);
    snprintf(fixed, max_len, "%s %s %s", trimmed_name, after_meta, metadata);
    return 1;
}

static int fix_character_name_typos(char *fixed, int max_len, const char *characters) {
    if (!characters || !*characters) return 0;
    char *colon = strchr(fixed, ':');
    char *meta = strchr(fixed, '{');
    if (!colon || (meta && colon > meta)) return 0;
    char name[256];
    size_t name_len = colon - fixed;
    if (name_len >= sizeof(name)) return 0;
    strncpy(name, fixed, name_len);
    trim(name);
    char chars_copy[1024];
    strncpy(chars_copy, characters, sizeof(chars_copy) - 1);
    chars_copy[sizeof(chars_copy) - 1] = '\0';
    char *candidates[64];
    int count = 0;
    char *token = strtok(chars_copy, ",");
    while (token && count < 63) {
        trim(token);
        candidates[count++] = token;
        token = strtok(NULL, ",");
    }
    candidates[count] = NULL;
    const char *correct = find_similar(name, (const char **) candidates);
    if (correct) {
        char rest[512];
        strcpy(rest, colon);
        snprintf(fixed, max_len, "%s%s", correct, rest);
        return 1;
    }
    return 0;
}

// Try to fix a line and return the fixed version
static int fix_line(const char *original, char *fixed, int max_len, const char *characters) {
    strncpy(fixed, original, max_len - 1);
    fixed[max_len - 1] = '\0';
    if (*fixed == '\0' || strncmp(fixed, "//", 2) == 0) return 0;
    return fix_header_typos(fixed, max_len) ||
           fix_metadata_typos(fixed, max_len) ||
           fix_missing_space_after_colon(fixed, max_len) ||
           fix_metadata_position(fixed, max_len) ||
           fix_character_name_typos(fixed, max_len, characters);
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
        const char result = compile(filename, 0);
        if (result > 0) {
            printf("\033[1;31mAuto-fix not possible, please fix manually\033[0m\n");
            return 1;
        }
        printf("\033[1;32mNo fixes needed\033[0m\n");
        return 0;
    }

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
