#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FATAL_ERROR(text) ({fprintf(stderr, "\e[1;31mFATAL ERROR:\e[0m %s\n", text); exit(0);})

typedef struct line {
    char *lin;
    int len;
} Line;

typedef struct terminal_char {
    int cnt_lines;
    int val;
    int max_width;
    Line **line;
} Terminal_Char;

typedef struct terminal_font {
    Terminal_Char **c;
    int cnt_chars;
    char *name;
    int max_lines;
} Terminal_Font;

Terminal_Font *load_font(char *path);

void render_text(char *text, Terminal_Font *font);

