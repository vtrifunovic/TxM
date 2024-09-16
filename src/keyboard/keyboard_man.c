#include "keyboard_man.h"

// re-creates the defaut config file & assigns keys to default ord values
static void _make_and_return_defaults(KeyBinds *binds, char *env_path){
    FILE *fix = fopen(env_path, "w");
    if (!fix){
        FATAL_ERROR("No .txm folder or local config file found!\nPlease use install script if building from source\n");
        exit(0);
    }
    char text[] = "PLAY=p\nSKIP=[\nPREV=o\nQUIT=q\n";
    fprintf(fix, text);
    fclose(fix);
    binds->quit = 113;
    binds->playpause = 112;
    binds->prev = 111;
    binds->next = 91;
    binds->exit = false;
    binds->command = 0;
}

static void _rotate_buffer(char *buf){
    for(int i = 1; i < 5; i++){
        buf[i-1] = buf[i];
    }
}

static void _read_and_assign_binds(FILE *f, KeyBinds *binds){
    int c;
    char buf[6] = "xxxxx";
    while ((c=getc(f)) != EOF){
        _rotate_buffer(buf);
        buf[4] = c;
        if (strcmp(buf, "PLAY=")==0)
            binds->playpause = getc(f);
        if (strcmp(buf, "SKIP=")==0)
            binds->next = getc(f);
        if (strcmp(buf, "PREV=")==0)
            binds->prev = getc(f);
        if (strcmp(buf, "QUIT=")==0)
            binds->quit = getc(f);
    }
}

KeyBinds *init_keybinds(void){
    // tries to read local copy of txm config first
    char fname[] = "txm_config.cfg";
    KeyBinds *binds = (KeyBinds*)malloc(sizeof(KeyBinds));
    FILE *file = fopen(fname, "r");
    if (!file){
        // tries to read HOME copy of txm config 2nd
        char txm_folder[] = "/.txm/";
        char *env_file = malloc(strlen(fname) + strlen(getenv("HOME")) + strlen(txm_folder) + 2);
        strcpy(env_file, getenv("HOME"));
        strcat(env_file, txm_folder);
        strcat(env_file, fname);
        FILE *file2 = fopen(env_file, "r");
        // if both fail, create config file in HOME/.txm directory
        if (!file2){
            _make_and_return_defaults(binds, env_file);
            return binds;
        }
        _read_and_assign_binds(file2, binds);
        fclose(file2);
        return binds;
    }
    _read_and_assign_binds(file, binds);
    fclose(file);
    return binds;
}