// https://dbus.freedesktop.org/doc/dbus-specification.html
// https://specifications.freedesktop.org/mpris-spec/latest/Player_Interface.html
#include <string.h>
#include "dbus_parser.h"

DBus_Info info;

// oh you thought we were done iterating the values??
static void get_metadata_info_values(DBusMessageIter args, DBusMessageIter tmp, char *signal, long int_value, char **val){
    // if its a type string we can simply put it into signal char
    if (signal[0] == 's' || signal[0] == 'o'){
        dbus_message_iter_get_basic(&tmp, &signal);
    // im pretty sure this isnt properly handling this, just a temp bypass
    } else if (signal[0] == 'x'){
        dbus_message_iter_get_basic(&tmp, &int_value);
        strcpy(signal, "x");
    // if its an array value with string as next
    // guess what we do?? wild guess? yep we recurse into it to pull out the value
    } else if (signal[0] == 'a' && signal[1] == 's'){
        dbus_message_iter_recurse(&tmp, &args);
        dbus_message_iter_get_basic(&args, &signal);
    // another temp bypass for ignored values
    } else {
        signal = realloc(signal, 5);
        strcpy(signal, "NULL");
    }
    // finally yes we are appending dbus data into our own struct
    if (!val)
        *val = malloc(strlen(signal) + 1);
    else
        *val = realloc(*val, strlen(signal) + 1);
    strcpy(*val, signal);
}

// this was the point i realized why even the devs tell you not to use their api
void parse_dbus_metadata(DBusMessageIter args, DBusMessageIter sub, char *signal){
    long int_value = 0;
    DBusMessageIter tmp;
    char *key;
    // mpris metadata has standard amount you need to recurse into it
    // so just hardcoding these iterations in here
    dbus_message_iter_recurse(&args, &sub);
    while (1){
        // loop over all the metadata values assigning them to proper
        // dbus info values until theres none left to yoink
        dbus_message_iter_recurse(&sub, &args);
        dbus_message_iter_get_basic(&args, &key);
        dbus_message_iter_next(&args);
        dbus_message_iter_recurse(&args, &tmp);
        signal = dbus_message_iter_get_signature(&tmp);
        if (strcmp(key, "mpris:artUrl") == 0)
            get_metadata_info_values(args, tmp, signal, int_value, &info.cover_path);
        if (strcmp(key, "xesam:album") == 0)
            get_metadata_info_values(args, tmp, signal, int_value, &info.album_str);
        if (strcmp(key, "xesam:artist") == 0)
            get_metadata_info_values(args, tmp, signal, int_value, &info.artist_str);
        if (strcmp(key, "xesam:title") == 0)
            get_metadata_info_values(args, tmp, signal, int_value, &info.title_str);
        if (dbus_message_iter_has_next(&sub))
            dbus_message_iter_next(&sub);
        else
            break;
    }
    //free(key);
}

bool startsWith(char *a, char *b){
    if (strncmp(a, b, strlen(b)) == 0) return true;
    return false;
}

void _set_value_to_default(bool memexist, char **val, char *cpy){
    if (memexist){
        *val = realloc(*val, strlen(cpy));
        strcpy(*val, cpy);
        return;
    } 
    *val = malloc(strlen(cpy));
    strcpy(*val, cpy);
}

void _add_interface_to_list(char *if_name){
    __if *ifx = (__if *)malloc(sizeof(__if));
    if (!info.if_list){
        info.if_list = (__if **)malloc(sizeof(__if));
    } else {
        info.if_list_len++;
        info.if_list = realloc(info.if_list, info.if_list_len*sizeof(__if));
    }
    ifx->name = (char *)malloc(strlen(if_name)+1);
    strcpy(ifx->name, if_name);
    info.if_list[info.if_list_len-1] = ifx;
}

static void _set_default_values(){
    // if we dont find a valid cover path
    // set it to default "unknown.png" image
    if (!info.cover_path)
        _set_value_to_default(0, &info.cover_path, info.unknown_path);
    else if (strlen(info.cover_path)==0)
        _set_value_to_default(1, &info.cover_path, info.unknown_path);
    if (!info.album_str)
        _set_value_to_default(0, &info.album_str, "Unknown");
    else if (strlen(info.album_str) == 0)
        _set_value_to_default(1, &info.album_str, "Unknown");
}

// can you now see why default dbus is a pain?
void handle_container_values(DBusMessageIter args, DBusMessageIter sub, char *signal){
    // if the signal value is playback we iter into it to get if song is playing
    if (strcmp(signal, "PlaybackStatus") == 0){
        dbus_message_iter_next(&sub);
        dbus_message_iter_recurse(&sub, &args);
        dbus_message_iter_get_basic(&args, &signal);
        if (strcmp(signal, "Playing")==0)
            info.playing = true;
        else
            info.playing = false;
    // otherwise if its metadata we gotta go even deeper
    } else if (strcmp(signal, "Metadata") == 0){
        dbus_message_iter_next(&sub);
        dbus_message_iter_recurse(&sub, &args);
        parse_dbus_metadata(args, sub, signal);
    } else {
        while (dbus_message_iter_has_next(&sub)){
            dbus_message_iter_next(&sub);
            int type = dbus_message_iter_get_arg_type(&sub);
            if (type == 118)
                return;
            dbus_message_iter_get_basic(&sub, &signal);
            // adding player interface to our struct
            // add once to our list here
            if (startsWith(signal, "org.mpris.MediaPlayer2")){
                _add_interface_to_list(signal);
            }
        }
    }
    _set_default_values();
    return;
}

// parsing the signal idk 
void parse_dbus_signal(DBusMessageIter args, DBusMessageIter sub){
    DBusMessageIter tmp;
    char *signal;
    while (1){
        // recurse deeper into message
        dbus_message_iter_recurse(&args, &sub);
        //int type = dbus_message_iter_get_arg_type(&sub);
        // check the type that it is
        signal = dbus_message_iter_get_signature(&sub);
        // if its a string its a "container" with more data to extract
        // otherwise we need to go deeper into the message OwO
        if (signal[0] == 's' && strlen(signal) == 1){
            dbus_message_iter_get_basic(&sub, &signal);
            handle_container_values(args, sub, signal);
            return;
        }
        tmp = args;
        args = sub;
        sub = tmp;
    }
    free(signal);
}

// handles replys to request_info_from_dbus
void parse_replys(DBusMessageIter args, DBusMessageIter sub){
    char *signal;
    dbus_message_iter_recurse(&args, &sub);
    signal = dbus_message_iter_get_signature(&sub);
    // s for play/pause
    // a for metadata
    if (signal[0] == 's'){
        dbus_message_iter_get_basic(&sub, &signal);
        if (strcmp(signal, "Playing")==0)
            info.playing = true;
        else
            info.playing = false;
    } else {
        parse_dbus_metadata(sub, args, signal);
        _set_default_values();
        return;
    }
}

void _debug_if_list(){
    if (!info.if_list) return;
    for (int i = 0; i < info.if_list_len; i++){
        fprintf(stdout, "Interface[%d] name: %s\n", i, info.if_list[i]->name);
        fprintf(stdout, "\tid: %s\n", info.if_list[i]->id);
    }
}

// if its a 97 which is an array we parse it otherwise safely return
static void handle_message_variants(DBusMessageIter args){
    int type = dbus_message_iter_get_arg_type(&args);
    DBusMessageIter sub;
    switch (type){
        case 97:
            parse_dbus_signal(args, sub);
            return;
        case 118:
            parse_replys(args, sub);
        default:
            return;
    }
}

// actual message handling
DBusHandlerResult read_message(DBusConnection *connection, DBusMessage *msg, void *user_data){
    if (strcmp(info.player_id, dbus_message_get_sender(msg)) != 0) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    DBusMessageIter args;
    if (!dbus_message_iter_init(msg, &args)){
        fprintf(stderr, "Message has no args\n");
    } else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)){
        fprintf(stderr, "Argument is not a string\n");
        fprintf(stdout, "Type: %d\n", dbus_message_iter_get_arg_type(&args));
    } else {
        // once we get a valid message we iterate through all the arguments
        // till theres none left to check
        while (dbus_message_iter_has_next(&args)){
            handle_message_variants(args);
            dbus_message_iter_next(&args);
        }
        //fprintf(stdout, "Sender: %s\n", dbus_message_get_sender(msg));
    }
    //debug_dbus_info(info);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void _setup_unknown_path(void){
    char txm_folder[] = "/.txm/unknown.png";
    info.unknown_path = malloc(strlen(getenv("HOME")) + strlen(txm_folder) + 2);
    strcpy(info.unknown_path, getenv("HOME"));
    strcat(info.unknown_path, txm_folder);
}

// sets up connection between this prog and mpris MediaPlayer2
// MediaPlayer2 is the default audio connection
DBusConnection *setup_dbus_connection(char *_path, char *_interface){
    DBusConnection *connection = NULL;
    DBusError error;
    DBusObjectPathVTable table;
    table.message_function = read_message;
    table.unregister_function = NULL;

    dbus_error_init(&error);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    dbus_error_exit(error);
    dbus_connection_try_register_object_path(
        connection,
        _path,
        &table,
        NULL,
        &error
    );
    dbus_error_exit(error);
    // important: this is what allows the code to interact with properites of dbus
    dbus_bus_add_match(connection, _interface, &error);
    dbus_error_exit(error);
    //dbus_bus_request_name(connection, "org.mpris.MediaPlayer2.txxm", DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    dbus_error_exit(error);
    dbus_connection_flush(connection);
    _setup_unknown_path();
    // this is an init thing too so imma put it here
    info.if_list_len = 1;
    return connection;
}

// returns the found info from dbus to use in ncurses_display.c
DBus_Info get_dbus_info(void){
    return info;
}

// probing mediaplayer2 to get back play state and metadata
static void request_info_from_dbus(DBusConnection *conn, char *message, char *chosen_interface, int if_idx){
    if (strcmp(message, "Metadata")==0 && info.title_str){
        strcpy(info.album_str, "");
        strcpy(info.title_str, "");
        strcpy(info.cover_path, "");
        strcpy(info.artist_str, "");
    }
    DBusMessage *msg;
    DBusMessageIter iter;
    char *dbusPath = malloc(31);
    strcpy(dbusPath, "org.mpris.MediaPlayer2.Player");
    // yoinked from dbus-send

    // dbus-send --print-reply --session 
    // --dest=org.mpris.MediaPlayer2.firefox.instance2869 
    // /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get 
    // string:'org.mpris.MediaPlayer2.Player' string:'PlaybackStatus'
    msg = dbus_message_new_method_call(NULL, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    dbus_message_set_auto_start(msg, TRUE);

    if (msg == NULL){
        fprintf(stderr, "Message failed to create\n");
    }
    // setting destination to mediaplayer instance
    if (!dbus_message_set_destination(msg, chosen_interface))
        fprintf(stderr, "%s interface not found\n", chosen_interface);

    dbus_message_iter_init_append(msg, &iter);
    // appending the mediaplayer2 variable we want to target
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &dbusPath);
    // appending the data we want to target from it
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &message);
    
    // sending the message with a reply block
    DBusError error;
    dbus_error_init(&error);

    DBusMessage *reply;
    reply = dbus_connection_send_with_reply_and_block(conn, msg, 20, &error);
    if (dbus_error_is_set(&error)){
        fprintf(stderr, "Error: %s: %s\n", error.name, error.message);
    } else if (reply){
        DBusMessageIter args;
        dbus_message_iter_init(reply, &args);
        if (if_idx >= 0){
            const char *sender_name = dbus_message_get_sender(reply);
            info.if_list[if_idx]->id = malloc(strlen(sender_name)+1);
            strcpy(info.if_list[if_idx]->id, sender_name);
        }
        handle_message_variants(args);
    } else {
        fprintf(stderr, "No reply recieved");
    }
    dbus_connection_flush(conn);
}


// gets exact mediaplayer2 instance that the audio is coming from
void get_dbus_player_instances(DBusConnection *conn){
    DBusMessage *msg;
    // messaging main DBus to get all connections
    msg = dbus_message_new_method_call(NULL, "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    if (msg == NULL){
        fprintf(stderr, "Message failed to create\n");
    }
    // setting destination to default DBus
    if (!dbus_message_set_destination(msg, "org.freedesktop.DBus"))
        fprintf(stderr, "org.freedesktop.DBus not found\n");
    
    // sending the message with a reply block
    DBusMessage *reply;
    DBusError error;
    dbus_error_init(&error);
    reply = dbus_connection_send_with_reply_and_block(conn, msg, 20, &error);
    if (dbus_error_is_set(&error)){
        fprintf(stderr, "Error: %s: %s\n", error.name, error.message);
    }
    // parse the reply with custom function
    if (reply){
        DBusMessageIter args;
        dbus_message_iter_init(reply, &args);
        handle_message_variants(args);
        // here we have our list, so lets send a hello to each of them to get their 
        // id and general data
        for (int x = 0; x < info.if_list_len; x++){
            request_info_from_dbus(conn, "PlaybackStatus", info.if_list[x]->name, x);
        }
        //_debug_if_list();
        // set last value to current player on init
        info.player_interface = malloc(strlen(info.if_list[info.if_list_len-1]->name)+1);
        info.player_id = malloc(strlen(info.if_list[info.if_list_len-1]->id)+1);

        strcpy(info.player_interface, info.if_list[info.if_list_len-1]->name);
        strcpy(info.player_id, info.if_list[info.if_list_len-1]->id);

        request_info_from_dbus(conn, "Metadata", info.player_interface, -1);
        info.if_curr_idx = info.if_list_len-1;
        //printf("%s %s", info.player_interface, info.if_list[0]->name);
    }
    dbus_connection_flush(conn);
}

// this is how pause/skip/prev is sent to mediaplayer
void send_dbus_info(DBusConnection *conn, char *message){
    DBusMessage *msg;
    // yoinked from dbus-monitor
    // path=/org/mpris/MediaPlayer2; interface=org.mpris.MediaPlayer2.Player; member=PlayPause
    msg = dbus_message_new_method_call(NULL, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", message);
    if (msg == NULL){
        fprintf(stderr, "Message failed to create\n");
    }
    // setting destination to mediaplayer instance
    if (!dbus_message_set_destination(msg, info.player_interface))
        fprintf(stderr, "%s interface not found\n", info.player_interface);
    
    // sending the message with a reply block
    DBusError error;
    dbus_error_init(&error);
    // we dont care about return signal. Just send and cast to void
    (void)dbus_connection_send_with_reply_and_block(conn, msg, 20, &error);
    if (dbus_error_is_set(&error)){
        //fprintf(stderr, "Error: %s: %s\n", error.name, error.message);
    }
    dbus_connection_flush(conn);
}

void switch_dbus_interface(DBusConnection *conn){
    info.if_curr_idx++;
    if (info.if_curr_idx >= info.if_list_len) info.if_curr_idx = 0;
    info.player_interface = realloc(info.player_interface, strlen(info.if_list[info.if_curr_idx]->name)+1);
    info.player_id = realloc(info.player_id, strlen( info.if_list[info.if_curr_idx]->id)+1);
    strcpy(info.player_interface, info.if_list[info.if_curr_idx]->name);
    strcpy(info.player_id, info.if_list[info.if_curr_idx]->id);
    request_info_from_dbus(conn, "PlaybackStatus", info.player_interface, -1);
    request_info_from_dbus(conn, "Metadata", info.player_interface, -1);
}

void check_info_and_send(KeyBinds *binds, DBusConnection *conn){
    if (binds->command == 0) // no message to send
        return;
    if (binds->command == 1)
        send_dbus_info(conn, "PlayPause");
    if (binds->command == 2)
        send_dbus_info(conn, "Next");
    if (binds->command == 3)
        send_dbus_info(conn, "Previous");
    if (binds->command == 5)
        switch_dbus_interface(conn);
    binds->command = 0;
}