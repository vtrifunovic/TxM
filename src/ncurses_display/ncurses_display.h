#pragma once
#include <curses.h>
#include <sys/ioctl.h>
#include "../info_structs.h"

void init_screen(bool *color, bool *big_text);

void handle_inputs(KeyBinds *binds);

void display_song_metadata(DBus_Info info, bool big_text);

void render_album_cover(DBus_Info info, bool color);