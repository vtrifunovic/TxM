#include <stdio.h>
#include <string.h>
#include "keyboard/keyboard_man.h"
#include "dbus_parser/dbus_parser.h"
#include "ncurses_display/ncurses_display.h"


static void _get_args(char *arg, bool *col, char **font){
    if (strcmp(arg, "--color")==0){
        *col = true;
    }
    if (strncmp(arg, "--font=", 7)==0){
        *font = (char *)malloc(strlen(arg)-7);
        arg += 7;
        strcpy(*font, arg);
    }
}

int main(int argc, char *argv[]){
    bool color = false;
    char *font_name = malloc(2);
    font_name[0] = '\n';
    if (argc > 1){
        for (int i = 1; i < argc; i++){
            _get_args(argv[i], &color, &font_name);
        }
    }
    DBusConnection *connection = setup_dbus_connection("/org/mpris/MediaPlayer2", "interface=org.freedesktop.DBus.Properties");
    get_dbus_player_instances(connection);
    init_screen(&color, font_name);
    DBus_Info main_info;
    KeyBinds *binds = init_keybinds();
    while (!binds->exit){
        dbus_connection_read_write_dispatch(connection, 100);
        main_info = get_dbus_info();
        render_album_cover(main_info, color);
        display_song_metadata(main_info, font_name);
        handle_inputs(binds);
        check_info_and_send(binds, connection);
    }
    endwin();
}