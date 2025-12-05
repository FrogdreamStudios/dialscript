// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

#pragma once

void verbose_header(const char *fullpath);

void verbose_footer(int line_num, int error);

void verbose_empty_line(int line_num);

void verbose_comment(int line_num, const char *text);

void verbose_scene(int line_num, int scene_num);

void verbose_dialog(int line_num, int dialog_num);

void verbose_level(int line_num, const char *val);

void verbose_location(int line_num, const char *val);

void verbose_characters(int line_num, const char *val);

void verbose_dialog_line(int line_num, const char *name, const char *text, const char *meta);

void verbose_error(int line_num, const char *message, const char *hint, const char *line_content, int error_pos);

void verbose_error_line(int line_num, const char *line_content);

void brief_error(int line_num, const char *message, const char *hint, const char *line_content, int error_pos);

void brief_result(int line_num, int error);
