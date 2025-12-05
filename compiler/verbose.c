// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#include "verbose.h"
#include <stdio.h>
#include <string.h>

#define TERMINAL_WIDTH 96
#define META_MAX_WIDTH 24

void verbose_header(const char *fullpath) {
    printf("\033[1;36mCompiling:\033[0m %s\n", fullpath);
}

void verbose_footer(const int line_num, const int error) {
    if (error == 0) {
        printf("\033[1;32mParsing completed:\033[0m %d lines processed\n", line_num);
    } else {
        printf("\033[1;31mParsing broken:\033[0m %d lines processed, %d error(s)\n", line_num, error);
    }
}

void verbose_empty_line(const int line_num) {
    printf("\033[90m%4d │ \033[0m\n", line_num);
}

void verbose_comment(const int line_num, const char *text) {
    printf("\033[90m%4d │\033[2m –%s\033[0m\n", line_num, text);
}

void verbose_scene(const int line_num, const int scene_num) {
    printf("\033[1;36m%4d │ ◉ Scene %d\033[0m\n", line_num, scene_num);
}

void verbose_dialog(const int line_num, const int dialog_num) {
    printf("\033[1;35m%4d │ ◆ Dialog %d\033[0m\n", line_num, dialog_num);
}

void verbose_level(const int line_num, const char *val) {
    printf("\033[90m%4d │   \033[36mLevel:\033[0m %s\n", line_num, val);
}

void verbose_location(const int line_num, const char *val) {
    printf("\033[90m%4d │   \033[36mLocation:\033[0m %s\n", line_num, val);
}

void verbose_characters(const int line_num, const char *val) {
    printf("\033[90m%4d │   \033[36mCharacters:\033[0m %s\n", line_num, val);
}

void verbose_dialog_line(const int line_num, const char *name, const char *text, const char *meta) {
    const int name_len = (int)strlen(name);
    const int max_text_width = TERMINAL_WIDTH - META_MAX_WIDTH - 4;
    const int text_len = (int)strlen(text);

    if (text_len <= max_text_width) {
        printf("\033[90m%4d │   \033[1;37m%s:\033[0m %s", line_num, name, text);
        for (int i = text_len; i < max_text_width; i++) printf(" ");
        printf("    ");
        if (meta) {
            printf("\033[33m%s\033[0m", meta);
        }
        printf("\n");
    } else {
        int wrap_pos = max_text_width;
        while (wrap_pos > 0 && text[wrap_pos] != ' ') wrap_pos--;
        if (wrap_pos == 0) wrap_pos = max_text_width;
        printf("\033[90m%4d │   \033[1;37m%s:\033[0m %.*s", line_num, name, wrap_pos, text);
        for (int i = wrap_pos; i < max_text_width; i++) printf(" ");
        printf("    ");
        if (meta) {
            printf("\033[33m%s\033[0m", meta);
        }
        printf("\n");

        const char *remaining = text + wrap_pos;
        while (*remaining == ' ') remaining++;

        while ((int)strlen(remaining) > 0) {
            int chunk_len = (int)strlen(remaining);
            if (chunk_len > max_text_width) {
                int pos = max_text_width;
                while (pos > 0 && remaining[pos] != ' ') pos--;
                if (pos == 0) pos = max_text_width;
                printf("\033[90m     │   ");
                for (int i = 0; i < name_len + 2; i++) printf(" ");
                printf("\033[0m%.*s\n", pos, remaining);
                remaining += pos;
                while (*remaining == ' ') remaining++;
            } else {
                printf("\033[90m     │   ");
                for (int i = 0; i < name_len + 2; i++) printf(" ");
                printf("\033[0m%s\n", remaining);
                break;
            }
        }
    }
}

void verbose_error(const int line_num, const char *message, const char *hint, const char *line_content, int error_pos) {
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

void brief_error(const int line_num, const char *message, const char *hint, const char *line_content, int error_pos) {
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
