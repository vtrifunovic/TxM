#include <stdio.h>
#include <string.h>
#include "keyboard/keyboard_man.h"
#include "dbus_parser/dbus_parser.h"
#include "ncurses_display/ncurses_display.h"

static void _get_args(char *arg, bool *col, bool *big_text){
    if (strcmp(arg, "--color")==0){
        *col = true;
    }
    if (strcmp(arg, "--big_text")==0){
        *big_text = true;
    }
}

int main(int argc, char *argv[]){
    bool color, big_text = false;
    if (argc > 1){
        for (int i = 1; i < argc; i++){
            _get_args(argv[i], &color, &big_text);
        }
    }
    DBusConnection *connection = setup_dbus_connection("/org/mpris/MediaPlayer2", "interface=org.freedesktop.DBus.Properties");
    get_dbus_player_instances(connection);
    init_screen(&color, &big_text);
    DBus_Info main_info;
    KeyBinds *binds = init_keybinds();
    while (!binds->exit){
        dbus_connection_read_write_dispatch(connection, 500);
        main_info = get_dbus_info();
        display_song_metadata(main_info, big_text);
        render_album_cover(main_info, color);
        handle_inputs(binds);
        check_info_and_send(binds, connection);
    }
    endwin();
}