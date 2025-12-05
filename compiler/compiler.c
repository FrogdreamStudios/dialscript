// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "compiler.h"
#include "parser.h"
#include "verbose.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int is_valid_character(const char *name, const char *characters) {
    if (!characters || !name) return 0;

    char chars_copy[1024];
    strncpy(chars_copy, characters, sizeof(chars_copy) - 1);
    chars_copy[sizeof(chars_copy) - 1] = '\0';

    char *token = strtok(chars_copy, ",");
    while (token) {
        // Trim whitespace
        while (*token && isspace(*token)) token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace(*end)) *end-- = '\0';

        if (strcmp(token, name) == 0) return 1;
        token = strtok(NULL, ",");
    }
    return 0;
}

char compile(const char *filename, int verbose) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "tests/%s", filename);

    FILE *file = fopen(fullpath, "r");
    if (!file) {
        fprintf(stderr, "Error! Cannot open file %s\n", filename);
        return 1;
    }

    char lines[1024][1024];
    char original_lines[1024][1024];
    int total_lines = 0;
    int max_width = 0;

    while (fgets(lines[total_lines], sizeof(lines[total_lines]), file)) {
        lines[total_lines][strcspn(lines[total_lines], "\n")] = 0;
        strcpy(original_lines[total_lines], lines[total_lines]);
        int len = (int)strlen(lines[total_lines]);
        if (len > max_width) max_width = len;
        total_lines++;
    }
    fclose(file);

    int line_width = 7 + max_width;
    if (line_width < 40) line_width = 40;

    if (verbose) verbose_header(fullpath, line_width);

    int in_scene = 0;
    int in_dialog = 0;
    int error = 0;
    char characters[1024] = {0};

    for (int i = 0; i < total_lines; i++) {
        int line_num = i + 1;
        ParsedLine pl = parse_line(lines[i]);

        switch (pl.type) {
            case LINE_EMPTY:
                if (verbose) verbose_empty_line(line_num);
                break;

            case LINE_COMMENT:
                if (verbose) verbose_comment(line_num, pl.value);
                break;

            case LINE_SCENE:
                if (pl.number <= 0) {
                    if (verbose) verbose_error(line_num, "Invalid scene number", "Scene numbers must be positive, e.g. [Scene.1]", original_lines[i], 7);
                    else brief_error(line_num, "Invalid scene number", "Scene numbers must be positive, e.g. [Scene.1]", original_lines[i], 7);
                    error++;
                } else {
                    in_scene = pl.number;
                    in_dialog = 0;
                    characters[0] = '\0';
                    if (verbose) verbose_scene(line_num, pl.number);
                }
                break;

            case LINE_DIALOG_HEADER:
                if (!in_scene) {
                    if (verbose) verbose_error(line_num, "Dialog outside scene", "Did you mean to add [Scene.1] before this?", original_lines[i], 0);
                    else brief_error(line_num, "Dialog outside scene", "Did you mean to add [Scene.1] before this?", original_lines[i], 0);
                    error++;
                } else if (pl.number <= 0) {
                    if (verbose) verbose_error(line_num, "Invalid dialog number", "Dialog numbers must be positive, e.g. [Dialog.1]", original_lines[i], 8);
                    else brief_error(line_num, "Invalid dialog number", "Dialog numbers must be positive, e.g. [Dialog.1]", original_lines[i], 8);
                    error++;
                } else {
                    in_dialog = 1;
                    if (verbose) verbose_dialog(line_num, pl.number);
                }
                break;

            case LINE_LEVEL:
                if (!in_scene) {
                    if (verbose) verbose_error(line_num, "Level outside scene", "Level must be inside a [Scene.N] block", original_lines[i], 0);
                    else brief_error(line_num, "Level outside scene", "Level must be inside a [Scene.N] block", original_lines[i], 0);
                    error++;
                } else if (verbose) {
                    verbose_level(line_num, pl.value);
                }
                break;

            case LINE_LOCATION:
                if (!in_scene) {
                    if (verbose) verbose_error(line_num, "Location outside scene", "Location must be inside a [Scene.N] block", original_lines[i], 0);
                    else brief_error(line_num, "Location outside scene", "Location must be inside a [Scene.N] block", original_lines[i], 0);
                    error++;
                } else if (verbose) {
                    verbose_location(line_num, pl.value);
                }
                break;

            case LINE_CHARACTERS:
                if (!in_scene) {
                    if (verbose) verbose_error(line_num, "Characters outside scene", "Characters must be inside a [Scene.N] block", original_lines[i], 0);
                    else brief_error(line_num, "Characters outside scene", "Characters must be inside a [Scene.N] block", original_lines[i], 0);
                    error++;
                } else {
                    strncpy(characters, pl.value, sizeof(characters) - 1);
                    characters[sizeof(characters) - 1] = '\0';
                    if (verbose) verbose_characters(line_num, pl.value);
                }
                break;

            case LINE_DIALOG:
                if (!in_dialog) {
                    if (verbose) verbose_error(line_num, "Dialog line outside dialog block", "Did you mean to add [Dialog.1] before this?", original_lines[i], 0);
                    else brief_error(line_num, "Dialog line outside dialog block", "Did you mean to add [Dialog.1] before this?", original_lines[i], 0);
                    error++;
                } else {
                    int line_has_error = 0;
                    // Check if character is in the characters list
                    if (characters[0] != '\0' && !is_valid_character(pl.name, characters)) {
                        if (verbose) verbose_error(line_num, "Unknown character", "This character is not in the Characters list", original_lines[i], 0);
                        else brief_error(line_num, "Unknown character", "This character is not in the Characters list", original_lines[i], 0);
                        error++;
                        line_has_error++;
                    }
                    if (pl.meta && !strchr(pl.meta, '}')) {
                        int meta_pos = (int)(pl.meta - lines[i]);
                        if (verbose) verbose_error(line_num, "Unclosed metadata", "Missing '}' at end of metadata block", original_lines[i], meta_pos);
                        else brief_error(line_num, "Unclosed metadata", "Missing '}' at end of metadata block", original_lines[i], meta_pos);
                        error++;
                        line_has_error++;
                    }
                    if (verbose && !line_has_error) verbose_dialog_line(line_num, pl.name, pl.text, pl.meta, line_width);
                }
                break;

            case LINE_UNKNOWN:
                if (in_dialog) {
                    // Inside dialog block, must have "Name: Text" format
                    if (verbose) verbose_error(line_num, "Missing colon in dialog", "Dialog lines must be: Name: Text", original_lines[i], 0);
                    else brief_error(line_num, "Missing colon in dialog", "Dialog lines must be: Name: Text", original_lines[i], 0);
                } else {
                    if (verbose) verbose_error(line_num, "Unknown syntax", "Check spelling or use valid format: [Scene.N], [Dialog.N], Name: Text", original_lines[i], 0);
                    else brief_error(line_num, "Unknown syntax", "Check spelling or use valid format: [Scene.N], [Dialog.N], Name: Text", original_lines[i], 0);
                }
                error++;
                break;

            case LINE_ERROR_EMPTY_NAME:
                if (verbose) verbose_error(line_num, "Empty character name", "Add a character name before ':', e.g. Alan: Hello", original_lines[i], 0);
                else brief_error(line_num, "Empty character name", "Add a character name before ':', e.g. Alan: Hello", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_EMPTY_TEXT:
                {
                    char *colon = strchr(original_lines[i], ':');
                    int colon_pos = colon ? (int)(colon - original_lines[i]) + 1 : 0;
                    if (verbose) verbose_error(line_num, "Empty dialog text", "Add dialog text after ':', e.g. Alan: Hello", original_lines[i], colon_pos);
                    else brief_error(line_num, "Empty dialog text", "Add dialog text after ':', e.g. Alan: Hello", original_lines[i], colon_pos);
                }
                error++;
                break;

            case LINE_ERROR_NO_SPACE_AFTER_COLON:
                {
                    char *colon = strchr(original_lines[i], ':');
                    int colon_pos = colon ? (int)(colon - original_lines[i]) + 1 : 0;
                    if (verbose) verbose_error(line_num, "Missing space after colon", "Format: Name: Text (space required after ':')", original_lines[i], colon_pos);
                    else brief_error(line_num, "Missing space after colon", "Format: Name: Text (space required after ':')", original_lines[i], colon_pos);
                }
                error++;
                break;

            case LINE_ERROR_INVALID_DIALOG_FORMAT:
                if (verbose) verbose_error(line_num, "Invalid dialog format", "Dialog must be: Name: Text", original_lines[i], 0);
                else brief_error(line_num, "Invalid dialog format", "Dialog must be: Name: Text", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_MISSING_COLON:
                if (verbose) verbose_error(line_num, "Missing colon", "Dialog lines must be: Name: Text", original_lines[i], 0);
                else brief_error(line_num, "Missing colon", "Dialog lines must be: Name: Text", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_UNKNOWN_CHARACTER:
                if (verbose) verbose_error(line_num, "Unknown character", "This character is not in the Characters list", original_lines[i], 0);
                else brief_error(line_num, "Unknown character", "This character is not in the Characters list", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_META_NOT_AT_END:
                {
                    char *meta = strchr(original_lines[i], '{');
                    int meta_pos = meta ? (int)(meta - original_lines[i]) : 0;
                    if (verbose) verbose_error(line_num, "Metadata must be at end of line", "Move {..} to the end: Name: Text {meta}", original_lines[i], meta_pos);
                    else brief_error(line_num, "Metadata must be at end of line", "Move {..} to the end: Name: Text {meta}", original_lines[i], meta_pos);
                }
                error++;
                break;

            case LINE_ERROR_TYPO_SCENE:
                if (verbose) verbose_error(line_num, "Invalid header format", "Did you mean [Scene.N]?", original_lines[i], 1);
                else brief_error(line_num, "Invalid header format", "Did you mean [Scene.N]?", original_lines[i], 1);
                error++;
                break;

            case LINE_ERROR_TYPO_DIALOG:
                if (verbose) verbose_error(line_num, "Invalid header format", "Did you mean [Dialog.N]?", original_lines[i], 1);
                else brief_error(line_num, "Invalid header format", "Did you mean [Dialog.N]?", original_lines[i], 1);
                error++;
                break;

            case LINE_ERROR_TYPO_LEVEL:
                if (verbose) verbose_error(line_num, "Unknown keyword", "Did you mean 'Level:'?", original_lines[i], 0);
                else brief_error(line_num, "Unknown keyword", "Did you mean 'Level:'?", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_TYPO_LOCATION:
                if (verbose) verbose_error(line_num, "Unknown keyword", "Did you mean 'Location:'?", original_lines[i], 0);
                else brief_error(line_num, "Unknown keyword", "Did you mean 'Location:'?", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_TYPO_CHARACTERS:
                if (verbose) verbose_error(line_num, "Unknown keyword", "Did you mean 'Characters:'?", original_lines[i], 0);
                else brief_error(line_num, "Unknown keyword", "Did you mean 'Characters:'?", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_UNCLOSED_BRACKET:
                {
                    int len = (int)strlen(original_lines[i]);
                    if (verbose) verbose_error(line_num, "Unclosed bracket", "Missing ']' at end of header", original_lines[i], len);
                    else brief_error(line_num, "Unclosed bracket", "Missing ']' at end of header", original_lines[i], len);
                }
                error++;
                break;
        }

        free_parsed_line(&pl);
    }

    if (verbose) {
        verbose_footer(line_width, total_lines, error);
    } else {
        brief_result(total_lines, error);
    }

    return error;
}

void print_result(const char result) {
    (void)result;
}
