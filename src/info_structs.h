#pragma once
#include <stdbool.h>
#include <stdint.h>

#define FATAL_ERROR(text) ({fprintf(stderr, "\e[1;31mFATAL ERROR:\e[0m %s\n", text); exit(0);})

typedef struct render_info{
    bool re_render;
    bool term_override;
    bool force_refresh;
    int playing;
    char *album_str;
    char *artist_str;
    int term_siz[2];
    int img_size;
} Render_Info;

typedef struct dbus_info{
    bool playing;
    char *album_str;
    char *artist_str;
    char *title_str;
    char *cover_path;
    char *player_interface;
} DBus_Info;

typedef struct keybinds{
    int quit;
    int playpause;
    int next;
    int prev;
    bool exit;
    int command;
} KeyBinds;