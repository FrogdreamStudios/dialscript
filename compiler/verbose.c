// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "verbose.h"
#include <stdio.h>

void verbose_header(const char *fullpath) {
    printf("\033[1;36mCompiling:\033[0m %s\n", fullpath);
}

void verbose_footer(int line_num, int error) {
    if (error == 0) {
        printf("\033[1;32mParsing completed:\033[0m %d lines processed\n", line_num);
    } else {
        printf("\033[1;31mParsing broken:\033[0m %d lines processed, %d error(s)\n", line_num, error);
    }
}

void verbose_empty_line(int line_num) {
    printf("\033[90m%4d │ \033[0m\n", line_num);
}

void verbose_comment(int line_num, const char *text) {
    printf("\033[90m%4d │\033[2m –%s\033[0m\n", line_num, text);
}

void verbose_scene(int line_num, int scene_num) {
    printf("\033[1;36m%4d │ ◉ Scene %d\033[0m\n", line_num, scene_num);
}

void verbose_dialog(int line_num, int dialog_num) {
    printf("\033[1;35m%4d │ ◆ Dialog %d\033[0m\n", line_num, dialog_num);
}

void verbose_level(const int line_num, const char *val) {
    printf("\033[90m%4d │   \033[36mLevel:\033[0m %s\n", line_num, val);
}

void verbose_location(int line_num, const char *val) {
    printf("\033[90m%4d │   \033[36mLocation:\033[0m %s\n", line_num, val);
}

void verbose_characters(int line_num, const char *val) {
    printf("\033[90m%4d │   \033[36mCharacters:\033[0m %s\n", line_num, val);
}

void verbose_dialog_line(int line_num, const char *name, const char *text, const char *meta) {
    if (meta) {
        printf("\033[90m%4d │   \033[1;37m%s:\033[0m %s \033[33m%s\033[0m\n",
               line_num, name, text, meta);
    } else {
        printf("\033[90m%4d │   \033[1;37m%s:\033[0m %s\n", line_num, name, text);
    }
}

void verbose_error(int line_num, const char *message, const char *hint, const char *line_content, int error_pos) {
    printf("\033[1;31m%4d │ ✗ \033[1;31m%s\033[0m\n", line_num, message);
    if (line_content) {
        printf("\033[90m     │   \033[31m%s\033[0m\n", line_content);
        if (error_pos >= 0) {
            printf("\033[90m     │   ");
            for (int i = 0; i < error_pos; i++) printf(" ");
            printf("\033[1;31m^\033[0m\n");
        }
    }
    if (hint) {
        printf("\033[90m     │   \033[1;90mHint:\033[0m \033[90m%s\033[0m\n", hint);
    }
}

void verbose_error_line(const int line_num, const char *line_content) {
    printf("\033[1;31m%4d │ ✗\033[0m \033[31m%s\033[0m\n", line_num, line_content);
}

void brief_error(int line_num, const char *message, const char *hint, const char *line_content, int error_pos) {
    printf("\033[1;31m%4d │ ✗ \033[1;31m%s\033[0m\n", line_num, message);
    if (line_content) {
        printf("\033[90m     │   \033[31m%s\033[0m\n", line_content);
        if (error_pos >= 0) {
            printf("\033[90m     │   ");
            for (int i = 0; i < error_pos; i++) printf(" ");
            printf("\033[1;31m^\033[0m\n");
        }
    }
    if (hint) {
        printf("\033[90m     │   \033[1;90mHint:\033[0m \033[90m%s\033[0m\n", hint);
    }
}

void brief_result(const int line_num, const int error) {
    if (error == 0) {
        printf("\033[1;32mParsing completed:\033[0m %d lines processed\n", line_num);
    } else {
        printf("\033[1;31mParsing broken:\033[0m %d lines processed, %d error(s)\n", line_num, error);
    }
    // TODO: add suggestion to use -v for more details
}
