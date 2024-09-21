#pragma once
#include <curses.h>
#include <sys/ioctl.h>
#include "../info_structs.h"

#define SET_TITLE(name) {fprintf(stdout, "\033]0;%s\007", name);}

void init_screen(bool *color, char *font_path);

void handle_inputs(KeyBinds *binds);

void display_song_metadata(DBus_Info info, char *font_path);

void render_album_cover(DBus_Info info, bool color);

void render_dbus_sources(DBus_Info info, KeyBinds binds);