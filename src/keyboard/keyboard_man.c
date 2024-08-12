#include "keyboard_man.h"

// re-creates the defaut config file & assigns keys to default ord values
static void _make_and_return_defaults(KeyBinds *binds){
    FILE *fix = fopen("config.cfg", "w");
    char text[] = "PLAY_PAUSE=p\nSKIP=[\nPREV=o\nQUIT=q\n";
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
    KeyBinds *binds = (KeyBinds*)malloc(sizeof(KeyBinds));
    FILE *file = fopen("config.cfg", "r");
    if (!file){
        _make_and_return_defaults(binds);
        return binds;
    }
    _read_and_assign_binds(file, binds);
    fclose(file);
    return binds;
}