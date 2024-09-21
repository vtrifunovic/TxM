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

typedef struct __if {
    char *name;
    char *id;
} __if;

typedef struct dbus_info{
    bool playing;
    char *album_str;
    char *artist_str;
    char *title_str;
    char *cover_path;
    char *unknown_path;
    char *player_interface; // current interface being used
    char *player_id; // current interface id being used
    __if **if_list; // list of all interfaces available
    int if_list_len;
    int if_curr_idx;
} DBus_Info;

typedef struct keybinds{
    int quit;
    int playpause;
    int next;
    int prev;
    int debug;
    int swch;
    bool exit;
    int command;
} KeyBinds;