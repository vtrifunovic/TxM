#pragma once
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include "../info_structs.h"

#define print_signal(arg) ({printf("Signal value: %s\n", dbus_message_iter_get_signature(&arg));})

#define dbus_error_exit(err)\
({\
    if (dbus_error_is_set(&error))\
    {\
        fprintf(stderr, "%s", error.message);\
        exit(0);\
    }\
})

#define debug_dbus_info(info)(\
{\
    printf("DBus Info:\n");\
    if (info.cover_path){\
        printf("\tCover: %s\n", info.cover_path);\
    }\
    if (info.album_str){\
        printf("\tAlbum: %s\n", info.album_str);\
    }\
    if (info.artist_str){\
        printf("\tArtist: %s\n", info.artist_str);\
    }\
    if (info.title_str){\
        printf("\tSong: %s\n", info.title_str);\
    }\
    if (info.playing){\
        printf("\tPlaystate: Playing!\n");\
    } else {\
        printf("\tPlaystate: Paused!\n");\
    }\
})

void parse_dbus_metadata(DBusMessageIter args, DBusMessageIter sub, char *signal);

DBusHandlerResult read_message(DBusConnection *connection, DBusMessage *msg, void *user_data);

DBusConnection *setup_dbus_connection(char *_path, char *_interface);

DBus_Info get_dbus_info(void);

void get_dbus_player_instances(DBusConnection *conn);

void check_info_and_send(KeyBinds *bind, DBusConnection *con);