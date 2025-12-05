// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#pragma once

// Line types
typedef enum {
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_SCENE,
    LINE_DIALOG_HEADER,
    LINE_LEVEL,
    LINE_LOCATION,
    LINE_CHARACTERS,
    LINE_DIALOG,
    LINE_UNKNOWN,
    LINE_ERROR_EMPTY_NAME,
    LINE_ERROR_EMPTY_TEXT,
    LINE_ERROR_NO_SPACE_AFTER_COLON,
    LINE_ERROR_INVALID_DIALOG_FORMAT,
    LINE_ERROR_MISSING_COLON,
    LINE_ERROR_UNKNOWN_CHARACTER,
    LINE_ERROR_UNCLOSED_BRACKET,
    LINE_ERROR_META_NOT_AT_END,
    LINE_ERROR_TYPO_SCENE,
    LINE_ERROR_TYPO_DIALOG,
    LINE_ERROR_TYPO_LEVEL,
    LINE_ERROR_TYPO_LOCATION,
    LINE_ERROR_TYPO_CHARACTERS,
    LINE_ERROR_EXTRA_SPACE_IN_HEADER,
    LINE_ERROR_EXTRA_SPACE_IN_METADATA,
    LINE_ERROR_LEADING_SPACE
} LineType;

// Parsed line data
typedef struct {
    LineType type;
    int number;     // Scene or dialog number
    char *value;    // For metadata (Level, Location, Characters)
    char *name;     // Character name in dialog
    char *text;     // Dialog text
    char *meta;     // Metadata block {..}
} ParsedLine;

// Parse a single line and return its type and data
ParsedLine parse_line(const char *line);

// Free parsed line resources (if any allocated)
void free_parsed_line(const ParsedLine *pl);
