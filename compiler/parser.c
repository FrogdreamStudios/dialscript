// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Helper: case-insensitive string compare
static int strcasecmp_partial(const char *s1, const char *s2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (tolower(s1[i]) != tolower(s2[i])) return 1;
    }
    return 0;
}

// Helper: check for typos in keywords
static int is_typo(const char *input, const char *expected) {
    size_t len_in = strlen(input);
    size_t len_exp = strlen(expected);

    // Too different in length
    if (len_in < len_exp - 2 || len_in > len_exp + 2) return 0;

    // Case-insensitive exact match is not a typo
    if (len_in == len_exp && strcasecmp_partial(input, expected, len_in) == 0) return 0;

    // Count matching characters
    int matches = 0;
    size_t min_len = len_in < len_exp ? len_in : len_exp;
    for (size_t i = 0; i < min_len; i++) {
        if (tolower(input[i]) == tolower(expected[i])) matches++;
    }

    // If more than 60% match, it's probably a typo
    return (matches * 100 / len_exp) > 60;
}

static char* skip_spaces(char *s) {
    while (*s && isspace(*s)) s++;
    return s;
}

static char* trim_end(char *s) {
    int len = strlen(s);
    while (len > 0 && isspace(s[len - 1])) {
        s[--len] = '\0';
    }
    return s;
}

ParsedLine parse_line(char *line) {
    ParsedLine pl = {0};

    // Empty line
    if (line[0] == '\0') {
        pl.type = LINE_EMPTY;
        return pl;
    }

    // Comment //
    if (strncmp(line, "//", 2) == 0) {
        pl.type = LINE_COMMENT;
        pl.value = line + 2;
        return pl;
    }

    // Scene header [Scene.x]
    int num;
    if (sscanf(line, "[Scene.%d]", &num) == 1) {
        pl.type = LINE_SCENE;
        pl.number = num;
        return pl;
    }

    // Dialog header [Dialog.x]
    if (sscanf(line, "[Dialog.%d]", &num) == 1) {
        pl.type = LINE_DIALOG_HEADER;
        pl.number = num;
        return pl;
    }

    // Metadata: Level, Location, Characters
    if (strncmp(line, "Level:", 6) == 0) {
        pl.type = LINE_LEVEL;
        pl.value = skip_spaces(line + 6);
        return pl;
    }
    if (strncmp(line, "Location:", 9) == 0) {
        pl.type = LINE_LOCATION;
        pl.value = skip_spaces(line + 9);
        return pl;
    }
    if (strncmp(line, "Characters:", 11) == 0) {
        pl.type = LINE_CHARACTERS;
        pl.value = skip_spaces(line + 11);
        return pl;
    }

    // Dialog line: Name: Text {meta}
    // Find the first colon, but it must be BEFORE any metadata block
    char *meta_start = strchr(line, '{');
    char *colon = strchr(line, ':');

    // If colon is inside metadata block or doesn't exist before it, it's an error
    if (colon && meta_start && colon > meta_start) {
        colon = NULL;  // Colon inside metadata doesn't count
    }

    if (colon) {
        // Check if there's a space after colon
        char after_colon = *(colon + 1);
        if (after_colon != ' ' && after_colon != '\0') {
            pl.type = LINE_ERROR_NO_SPACE_AFTER_COLON;
            return pl;
        }

        *colon = '\0';
        pl.name = skip_spaces(line);
        trim_end(pl.name);

        pl.text = skip_spaces(colon + 1);

        // Extract metadata
        char *meta = strchr(pl.text, '{');
        if (meta) {
            // Check if metadata is at the end (nothing after closing brace except whitespace)
            char *closing = strchr(meta, '}');
            if (closing) {
                char *after_meta = closing + 1;
                while (*after_meta && isspace(*after_meta)) after_meta++;
                if (*after_meta != '\0') {
                    // There's text after metadata - error!
                    pl.type = LINE_ERROR_META_NOT_AT_END;
                    return pl;
                }
            }

            pl.meta = meta;
            // Trim text before metadata
            char *text_end = meta - 1;
            while (text_end > pl.text && isspace(*text_end)) text_end--;
            *(text_end + 1) = '\0';
        }

        // Validate dialog line
        if (strlen(pl.name) == 0) {
            pl.type = LINE_ERROR_EMPTY_NAME;
            return pl;
        }
        if (strlen(pl.text) == 0) {
            pl.type = LINE_ERROR_EMPTY_TEXT;
            return pl;
        }

        pl.type = LINE_DIALOG;
        return pl;
    }

    // Check for typos in headers
    if (line[0] == '[') {
        char header[64] = {0};
        sscanf(line, "[%63[^.]", header);

        if (is_typo(header, "Scene")) {
            pl.type = LINE_ERROR_TYPO_SCENE;
            return pl;
        }
        if (is_typo(header, "Dialog")) {
            pl.type = LINE_ERROR_TYPO_DIALOG;
            return pl;
        }
    }

    // Check for typos in metadata keywords
    char keyword[64] = {0};
    char *colon_pos = strchr(line, ':');
    if (colon_pos && (colon_pos - line) < 64) {
        strncpy(keyword, line, colon_pos - line);
        keyword[colon_pos - line] = '\0';
        trim_end(keyword);

        if (is_typo(keyword, "Level")) {
            pl.type = LINE_ERROR_TYPO_LEVEL;
            return pl;
        }
        if (is_typo(keyword, "Location")) {
            pl.type = LINE_ERROR_TYPO_LOCATION;
            return pl;
        }
        if (is_typo(keyword, "Characters")) {
            pl.type = LINE_ERROR_TYPO_CHARACTERS;
            return pl;
        }
    }

    pl.type = LINE_UNKNOWN;
    return pl;
}

void free_parsed_line(const ParsedLine *pl) {
    (void)pl;
}
