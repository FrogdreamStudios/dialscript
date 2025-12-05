// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Skip whitespaces
static inline char *skip_ws(const char *s) {
    while (*s && isspace((unsigned char) *s)) s++;
    return (char *) s;
}

// Right trim
static inline char *rtrim(char *str) {
    if (!str || !*str) return str;
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char) *end)) *end-- = '\0';
    return str;
}

// Case-insensitive string compare for n characters
static inline bool strneq_ci(const char *a, const char *b, const size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (tolower((unsigned char) a[i]) != tolower((unsigned char) b[i])) return false;
        if (!b[i]) return true;
    }
    return true;
}

// Main parser
ParsedLine parse_line(const char *line) {
    ParsedLine pl = {0};
    pl.type = LINE_UNKNOWN;

    char *s = skip_ws(line);

    // Empty line
    if (!*s) {
        pl.type = LINE_EMPTY;
        return pl;
    }

    // Comment
    if (s[0] == '/' && s[1] == '/') {
        pl.type = LINE_COMMENT;
        pl.value = skip_ws(s + 2);
        return pl;
    }

    // Headers
    if (s[0] == '[') {
        char *p = s + 1;
        char *dot = NULL;
        char *close = strchr(p, ']');

        if (!close) {
            pl.type = LINE_ERROR_UNCLOSED_BRACKET;
            return pl;
        }

        // Find dot
        for (char *t = p; t < close; ++t) {
            if (*t == '.') {
                dot = t;
                break;
            }
        }

        if (!dot || dot == p || dot + 1 == close) {
            pl.type = LINE_ERROR_TYPO_SCENE;
        } else {
            const size_t prefix_len = dot - p;

            if (strneq_ci(p, "scene", prefix_len)) {
                int num;
                if (sscanf(dot + 1, "%d", &num) == 1 && num > 0) {
                    const char *after_num = skip_ws(dot + 1 + strspn(dot + 1, "0123456789"));
                    if (after_num < close) {
                        pl.type = LINE_ERROR_EXTRA_SPACE_IN_HEADER;
                        return pl;
                    }
                    pl.type = LINE_SCENE;
                    pl.number = num;
                    return pl;
                }
            } else if (strneq_ci(p, "dialog", prefix_len)) {
                int num;
                if (sscanf(dot + 1, "%d", &num) == 1 && num > 0) {
                    const char *after_num = skip_ws(dot + 1 + strspn(dot + 1, "0123456789"));
                    if (after_num < close) {
                        pl.type = LINE_ERROR_EXTRA_SPACE_IN_HEADER;
                        return pl;
                    }
                    pl.type = LINE_DIALOG_HEADER;
                    pl.number = num;
                    return pl;
                }
            }
        }

        pl.type = LINE_ERROR_TYPO_SCENE;
        if (strneq_ci(p, "dialog", 6)) pl.type = LINE_ERROR_TYPO_DIALOG;
        return pl;
    }

    // Metadata
    const char *colon = strchr(s, ':');
    if (colon && colon > s) {
        const size_t kw_len = colon - s;

        if (strneq_ci(s, "level", kw_len) && kw_len == 5) {
            pl.type = LINE_LEVEL;
            pl.value = skip_ws(colon + 1);
            return pl;
        }
        if (strneq_ci(s, "location", kw_len) && kw_len == 8) {
            pl.type = LINE_LOCATION;
            pl.value = skip_ws(colon + 1);
            return pl;
        }
        if (strneq_ci(s, "characters", kw_len) && kw_len == 10) {
            pl.type = LINE_CHARACTERS;
            pl.value = skip_ws(colon + 1);
            return pl;
        }

        // Typos in metadata keywords
        // TODO: improve typo detection
        if (strneq_ci(s, "leve", kw_len) || strneq_ci(s, "levl", kw_len)) {
            pl.type = LINE_ERROR_TYPO_LEVEL;
            return pl;
        }
        if (strneq_ci(s, "locatio", kw_len)) {
            pl.type = LINE_ERROR_TYPO_LOCATION;
            return pl;
        }
        if (strneq_ci(s, "character", kw_len)) {
            pl.type = LINE_ERROR_TYPO_CHARACTERS;
            return pl;
        }
    }

    // Dialog line
    char *meta_start = strchr(s, '{');
    char *real_colon = NULL;

    // Find the real colon (before metadata)
    if (meta_start) {
        for (char *c = s; c < meta_start; ++c) {
            if (*c == ':') {
                real_colon = c;
                break;
            }
        }
    } else {
        real_colon = strchr(s, ':');
    }

    if (real_colon) {
        // Spaces? Well, we don't allow leading spaces
        if (s != line && isspace((unsigned char) line[0])) {
            pl.type = LINE_ERROR_LEADING_SPACE;
            return pl;
        }

        // Detect missing space after colon
        const char *after_colon = real_colon + 1;
        if (*after_colon && !isspace((unsigned char) *after_colon)) {
            pl.type = LINE_ERROR_NO_SPACE_AFTER_COLON;
            return pl;
        }

        // Character name
        *real_colon = '\0';
        pl.name = s;
        rtrim(pl.name);

        if (pl.name[0] == '\0') {
            pl.type = LINE_ERROR_EMPTY_NAME;
            return pl;
        }

        // Dialog text
        pl.text = skip_ws(after_colon);

        // Check for dialog metadata
        if (meta_start) {
            char *close = strchr(meta_start, '}');
            if (!close) {
                pl.type = LINE_ERROR_UNCLOSED_BRACKET;
                pl.meta = meta_start;
                return pl;
            }
            const char *after_meta = skip_ws(close + 1);
            if (*after_meta) {
                pl.type = LINE_ERROR_META_NOT_AT_END;
                return pl;
            }
            // Trim text before metadata
            char *text_end = meta_start - 1;
            while (text_end > pl.text && isspace((unsigned char) *text_end)) text_end--;
            *(text_end + 1) = '\0';
            pl.meta = meta_start;
        }

        if (pl.text[0] == '\0') {
            pl.type = LINE_ERROR_EMPTY_TEXT;
            return pl;
        }

        pl.type = LINE_DIALOG;
        return pl;
    }

    // Everything else is unknown
    pl.type = LINE_UNKNOWN;
    return pl;
}

void free_parsed_line(const ParsedLine *pl) {
    (void) pl;
}
