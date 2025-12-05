// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "compiler.h"
#include "parser.h"
#include "verbose.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Helper functions
static void report_error_at(const int verbose, const int line, const char *msg, const char *hint, const char *line_content,
                            const int pos) {
    if (verbose) verbose_error(line, msg, hint, line_content, pos);
    else brief_error(line, msg, hint, line_content, pos);
}

// Check if character name is in the known list
static int char_known(const char *name, const char *list) {
    char buf[1024];
    strncpy(buf, list, sizeof(buf)-1);
    buf[sizeof(buf) - 1] = '\0';

    char *t = strtok(buf, ",");
    while (t) {
        while (isspace(*t)) t++;
        char *end = t + strlen(t) - 1;
        while (end >= t && isspace(*end)) *end-- = '\0';
        if (strcmp(t, name) == 0) return 1;
        t = strtok(NULL, ",");
    }
    return 0;
}

// Main compile function
char compile(const char *filename, const int verbose) {
    char path[1024];
    snprintf(path, sizeof(path), "tests/%s", filename);

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Cannot open: %s\n", filename);
        return 1;
    }

    char lines[1024][1024];
    char original_lines[1024][1024];
    int total_lines = 0;

    memset(lines, 0, sizeof(lines));
    memset(original_lines, 0, sizeof(original_lines));

    while (fgets(lines[total_lines], 1024, f)) {
        lines[total_lines][strcspn(lines[total_lines], "\n")] = '\0';
        strcpy(original_lines[total_lines], lines[total_lines]);
        total_lines++;
    }

    fclose(f);
    if (verbose) verbose_header(path);

    int error = 0;
    int scene = 0, dialog = 0;
    char characters[1024] = {0};

    int has_scene = 0, has_level = 0, has_location = 0, has_chars = 0;

    // Define error reporting macros
#define fail(msg, hint)         do { report_error_at(verbose, line_num, msg, hint, original_lines[i], 0); error++; } while(0)
#define fail_at(msg, hint, pos) do { report_error_at(verbose, line_num, msg, hint, original_lines[i], pos); error++; } while(0)
#define fail_final(msg, hint)   do { report_error_at(verbose, total_lines, msg, hint, NULL, 0); error++; } while(0)

    // Parse each line
    for (int i = 0; i < total_lines; i++) {
        const int line_num = i + 1;
        ParsedLine p = parse_line(lines[i]);

        switch (p.type) {
            case LINE_EMPTY:
                if (dialog && i + 1 < total_lines) {
                    ParsedLine next = parse_line(lines[i + 1]);
                    const int bad = next.type != LINE_DIALOG_HEADER && next.type != LINE_COMMENT;
                    free_parsed_line(&next);
                    if (bad)
                        fail("Empty line inside dialog block", "Remove empty lines between dialog lines");
                }
                if (verbose) verbose_empty_line(line_num);
                break;

            case LINE_COMMENT:
                if (verbose) verbose_comment(line_num, p.value);
                break;

            case LINE_SCENE:
                if (has_scene)
                    fail("Only one [Scene.X] allowed", "Remove extra scene declarations");
                else if (p.number <= 0)
                    fail_at("Scene number must be > 0", "Use [Scene.1], [Scene.2], etc.", 7);
                else {
                    scene = p.number;
                    dialog = 0;
                    characters[0] = '\0';
                    has_scene = 1;
                    if (verbose) verbose_scene(line_num, p.number);
                }
                break;

            case LINE_DIALOG_HEADER:
                if (!scene)
                    fail("Dialog without [Scene.X]", "Add [Scene.1] before this dialog");
                else if (p.number <= 0)
                    fail_at("Dialog number must be > 0", "Use [Dialog.1], [Dialog.2], etc.", 8);
                else {
                    dialog = 1;
                    if (verbose) verbose_dialog(line_num, p.number);
                }
                break;

            case LINE_LEVEL:
                if (!scene)
                    fail("Level outside scene", "Move Level: inside [Scene.X] block");
                else if (dialog)
                    fail("Level after dialog", "Move Level: before [Dialog.X]");
                else if (has_level)
                    fail("Duplicate Level", "Remove extra Level definition");
                else {
                    has_level = 1;
                    if (verbose) verbose_level(line_num, p.value);
                }
                break;

            case LINE_LOCATION:
                if (!scene)
                    fail("Location outside scene", "Move Location: inside [Scene.X] block");
                else if (dialog)
                    fail("Location after dialog", "Move Location: before [Dialog.X]");
                else if (has_location)
                    fail("Duplicate Location", "Remove extra Location definition");
                else {
                    has_location = 1;
                    if (verbose) verbose_location(line_num, p.value);
                }
                break;

            case LINE_CHARACTERS:
                if (!scene)
                    fail("Characters outside scene", "Move Characters: inside [Scene.X] block");
                else if (dialog)
                    fail("Characters after dialog", "Move Characters: before [Dialog.X]");
                else if (has_chars)
                    fail("Duplicate Characters", "Remove extra Characters definition");
                else {
                    strncpy(characters, p.value, 1023);
                    has_chars = 1;
                    if (verbose) verbose_characters(line_num, p.value);
                }
                break;

            case LINE_DIALOG:
                if (!dialog) {
                    fail("Stray dialog line", "Add [Dialog.1] before this line");
                    break;
                }

                if (characters[0] && !char_known(p.name, characters))
                    fail("Unknown character", "Add this character to Characters");

                if (p.meta && !strchr(p.meta, '}'))
                    fail_at("Missing '}' in metadata", "Close metadata with '}'", (int)(p.meta - lines[i]));

                if (verbose) verbose_dialog_line(line_num, p.name, p.text, p.meta);
                break;

            // Errors
            case LINE_ERROR_EMPTY_NAME: fail("Empty name before ':'", "Add character name, e.g. Alan: Hello");
                break;
            case LINE_ERROR_MISSING_COLON: fail("Missing ':' in dialog", "Use format: Name: Text");
                break;
            case LINE_ERROR_INVALID_DIALOG_FORMAT: fail("Wrong dialog format", "Use format: Name: Text");
                break;
            case LINE_ERROR_TYPO_SCENE: fail_at("Did you mean [Scene.N]?", "Check spelling", 1);
                break;
            case LINE_ERROR_TYPO_DIALOG: fail_at("Did you mean [Dialog.N]?", "Check spelling", 1);
                break;
            case LINE_ERROR_TYPO_LEVEL: fail("Did you mean 'Level:'?", "Check spelling");
                break;
            case LINE_ERROR_TYPO_LOCATION: fail("Did you mean 'Location:'?", "Check spelling");
                break;
            case LINE_ERROR_TYPO_CHARACTERS: fail("Did you mean 'Characters:'?", "Check spelling");
                break;
            case LINE_ERROR_UNCLOSED_BRACKET: fail_at("Missing ']'", "Close header with ']'", (int)strlen(lines[i]));
                break;
            case LINE_ERROR_EXTRA_SPACE_IN_HEADER: fail("Extra space in header",
                                                        "Use [Scene.1] or [Dialog.1] without spaces");
                break;
            case LINE_ERROR_EXTRA_SPACE_IN_METADATA: fail("Extra space before ':'",
                                                          "Use 'Level:', 'Location:', 'Characters:' without spaces");
                break;
            case LINE_ERROR_LEADING_SPACE: fail("Leading space in dialog line",
                                                "Character name must start at the beginning of the line");
                break;
            case LINE_UNKNOWN:
                if (dialog)
                    fail("Invalid line in dialog", "Use format: Name: Text");
                else
                    fail("Unknown syntax", "Check spelling or use: [Scene.N], [Dialog.N], Name: Text");
                break;

            default:
                break;
        }

        free_parsed_line(&p);
    }

    // Final checks (missing of required parts)
    if (!has_scene)
        fail_final("Missing [Scene.X]", "Add [Scene.1] at the beginning of file");
    if (!has_level)
        fail_final("Missing Level", "Add 'Level: N' after [Scene.X]");
    if (!has_location)
        fail_final("Missing Location", "Add 'Location: name' after [Scene.X]");
    if (!has_chars)
        fail_final("Missing Characters", "Add 'Characters: Name1, Name2' after [Scene.X]");

// Undefine error reporting macros
#undef fail
#undef fail_at
#undef fail_final

    // Final verbose output
    if (verbose) verbose_footer(total_lines, error);
    else brief_result(total_lines, error);

    // Return status
    return (char) error;
}

void print_result(const char result) { (void) result; }
