// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "compiler.h"
#include "parser.h"
#include "verbose.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define REPORT_ERROR(title, msg) { report_error(verbose, line_num, title, msg, original_lines[i], 0); error++; }
#define REPORT_COLON_ERROR(title, msg) { char *colon = strchr(original_lines[i], ':'); int colon_pos = colon ? (int)(colon - original_lines[i]) + 1 : 0; report_error(verbose, line_num, title, msg, original_lines[i], colon_pos); error++; }
#define REPORT_META_ERROR(title, msg) { char *meta = strchr(original_lines[i], '{'); int meta_pos = meta ? (int)(meta - original_lines[i]) : 0; report_error(verbose, line_num, title, msg, original_lines[i], meta_pos); error++; }
#define REPORT_LEN_ERROR(title, msg) { int len = (int)strlen(original_lines[i]); report_error(verbose, line_num, title, msg, original_lines[i], len); error++; }

static int is_valid_character(const char *name, const char *characters) {
    if (!characters || !name) return 0;

    char chars_copy[1024];
    strncpy(chars_copy, characters, sizeof(chars_copy) - 1);
    chars_copy[sizeof(chars_copy) - 1] = '\0';

    char *token = strtok(chars_copy, ",");
    while (token) {
        while (*token && isspace(*token)) token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace(*end)) *end-- = '\0';

        if (strcmp(token, name) == 0) return 1;
        token = strtok(NULL, ",");
    }
    return 0;
}

static void report_error(int verbose, int line_num, const char *title, const char *msg, const char *line, int pos) {
    if (verbose) verbose_error(line_num, title, msg, line, pos);
    else brief_error(line_num, title, msg, line, pos);
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

    while (fgets(lines[total_lines], sizeof(lines[total_lines]), file)) {
        lines[total_lines][strcspn(lines[total_lines], "\n")] = 0;
        strcpy(original_lines[total_lines], lines[total_lines]);
        total_lines++;
    }
    fclose(file);

    if (verbose) verbose_header(fullpath);

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
                    report_error(verbose, line_num, "Invalid scene number",
                                 "Scene numbers must be positive, e.g. [Scene.1]", original_lines[i], 7);
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
                    REPORT_ERROR("Dialog outside scene", "Did you mean to add [Scene.1] before this?");
                } else if (pl.number <= 0) {
                    report_error(verbose, line_num, "Invalid dialog number",
                                 "Dialog numbers must be positive, e.g. [Dialog.1]", original_lines[i], 8);
                    error++;
                } else {
                    in_dialog = 1;
                    if (verbose) verbose_dialog(line_num, pl.number);
                }
                break;

            case LINE_LEVEL:
                if (!in_scene) {
                    REPORT_ERROR("Level outside scene", "Level must be inside a [Scene.N] block");
                } else if (verbose) {
                    verbose_level(line_num, pl.value);
                }
                break;

            case LINE_LOCATION:
                if (!in_scene) {
                    REPORT_ERROR("Location outside scene", "Location must be inside a [Scene.N] block");
                } else if (verbose) {
                    verbose_location(line_num, pl.value);
                }
                break;

            case LINE_CHARACTERS:
                if (!in_scene) {
                    REPORT_ERROR("Characters outside scene", "Characters must be inside a [Scene.N] block");
                } else {
                    strncpy(characters, pl.value, sizeof(characters) - 1);
                    characters[sizeof(characters) - 1] = '\0';
                    if (verbose) verbose_characters(line_num, pl.value);
                }
                break;

            case LINE_DIALOG:
                if (!in_dialog) {
                    REPORT_ERROR("Dialog line outside dialog block", "Did you mean to add [Dialog.1] before this?");
                } else {
                    int line_has_error = 0;
                    if (characters[0] != '\0' && !is_valid_character(pl.name, characters)) {
                        REPORT_ERROR("Unknown character", "This character is not in the Characters list");
                        line_has_error++;
                    }
                    if (pl.meta && !strchr(pl.meta, '}')) {
                        int meta_pos = (int) (pl.meta - lines[i]);
                        report_error(verbose, line_num, "Unclosed metadata", "Missing '}' at end of metadata block",
                                     original_lines[i], meta_pos);
                        error++;
                        line_has_error++;
                    }
                    if (verbose && !line_has_error) verbose_dialog_line(line_num, pl.name, pl.text, pl.meta);
                }
                break;

            case LINE_UNKNOWN:
                if (in_dialog) {
                    // Inside dialog block, must have "Name: Text" format
                    REPORT_ERROR("Missing colon in dialog", "Dialog lines must be: Name: Text");
                } else {
                    REPORT_ERROR("Unknown syntax",
                                 "Check spelling or use valid format: [Scene.N], [Dialog.N], Name: Text");
                }
                break;

            case LINE_ERROR_EMPTY_NAME:
                REPORT_ERROR("Empty character name", "Add a character name before ':', e.g. Alan: Hello");
                break;

            case LINE_ERROR_INVALID_DIALOG_FORMAT:
                REPORT_ERROR("Invalid dialog format", "Dialog must be: Name: Text");
                break;

            case LINE_ERROR_MISSING_COLON:
                REPORT_ERROR("Missing colon", "Dialog lines must be: Name: Text");
                break;

            case LINE_ERROR_UNKNOWN_CHARACTER:
                REPORT_ERROR("Unknown character", "This character is not in the Characters list");
                break;

            case LINE_ERROR_TYPO_SCENE:
                report_error(verbose, line_num, "Invalid header format", "Did you mean [Scene.N]?", original_lines[i],
                             1);
                error++;
                break;

            case LINE_ERROR_TYPO_DIALOG:
                report_error(verbose, line_num, "Invalid header format", "Did you mean [Dialog.N]?", original_lines[i],
                             1);
                error++;
                break;

            case LINE_ERROR_TYPO_LEVEL:
                REPORT_ERROR("Unknown keyword", "Did you mean 'Level:'?");
                break;

            case LINE_ERROR_TYPO_LOCATION:
                report_error(verbose, line_num, "Unknown keyword", "Did you mean 'Location:'?", original_lines[i], 0);
                error++;
                break;

            case LINE_ERROR_TYPO_CHARACTERS:
                REPORT_ERROR("Unknown keyword", "Did you mean 'Characters:'?");
                break;

            case LINE_ERROR_UNCLOSED_BRACKET:
                REPORT_LEN_ERROR("Unclosed bracket", "Missing ']' at end of header");
                break;

            default:
                break;
        }

        free_parsed_line(&pl);
    }

    if (verbose) {
        verbose_footer(total_lines, error);
    } else {
        brief_result(total_lines, error);
    }

    return error;
}

void print_result(const char result) {
    (void) result;
}
