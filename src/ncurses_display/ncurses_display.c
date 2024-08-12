#include <string.h>
#include <stdlib.h>
#include "ncurses_display.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../text_render/text.h"

Render_Info ri;
// idk why i did it like this please ignore
char g_char[] = {'x', '-', '?', ';', ' ', '.', ':', '=', '+'};
int render_count;
Terminal_Font *font;

// allocates memory for our render info data
// if type 1 means data unallocated have to initialize
// if type 2 means data allocated just have to resize
static void alloc_mem_struct(char **str, char *string, int type){
    if (type == 1)
        *str = (char *)malloc(strlen(string)+1);
    else if (type == 2)
        *str = (char *)realloc(*str, strlen(string)+1);
    else // god help you if you somehow get here
        exit(0);
    strcpy(*str, string);
}

// uses ioctl to get size of terminal in amount of rows & cols
// uses divisor for formatting
static void get_terminal_size(int *rows, int *cols, float divisor){
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    if (ri.term_siz[0] != w.ws_row || ri.term_siz[1] != w.ws_col){
        ri.term_override = true;
    }
    *rows = w.ws_row/divisor;
    *cols = w.ws_col/divisor;
    ri.term_siz[0] = w.ws_row;
    ri.term_siz[1] = w.ws_col;
}

// initializes the color sqaures for album covers using the basic
// ncurses color palet
void init_basic_colors(void){
    init_color(COLOR_BLACK, 0, 0, 0);
    init_pair(1, COLOR_BLACK, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_RED);
    init_pair(3, COLOR_GREEN, COLOR_GREEN);
    init_pair(4, COLOR_BLUE, COLOR_BLUE);
    init_pair(5, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(7, COLOR_CYAN, COLOR_CYAN);
    init_pair(8, COLOR_WHITE, COLOR_WHITE);
}

// got tired of copy pasting this, calculates the id of the rgb that 
// the pixel should be
int calc_equation(int r, int g, int b){
    return r+g*6+b*36+1;
}

// if terminal has support for 256 colors
// initializes 5^3 color options for code to choose from
// with each having a unique id to reference
void init_mega_colors(void){
    for (int r = 0; r < 6; r++){
        for (int g = 0; g < 6; g++){
            for (int b = 0; b < 6; b++){
                int id = calc_equation(r, g, b);
                init_color(id, r*300, g*300, b*300);
                init_pair(id, id, id);
            }
        }
    }
}

// standard screen initializations along with custom color additions
void init_screen(bool *color, bool *big_text){
    initscr();
    noecho();
    use_default_colors();
    if (has_colors() == TRUE){
        start_color();
        if (can_change_color()){
            init_mega_colors();
        } else{
            init_basic_colors();
        }
    } else{
        *color = false;
    }
    if (big_text) font = load_font("fonts/small.k9");
    if (!font){
        *big_text = false;
    }
    // safe tea memory allocations init the char* memory slot
    alloc_mem_struct(&ri.album_str, "None", 1);
    alloc_mem_struct(&ri.artist_str, "None", 1);
    ri.playing = 2;
    curs_set(0);
    timeout(1);
}

// if q then quit, later will include keybinds
void handle_inputs(KeyBinds *binds){
    int ch = getch();
    if (ch == binds->quit)
        binds->exit = true;
    if (ch == binds->playpause)
        binds->command = 1;
    if (ch == binds->next)
        binds->command = 2;
    if (ch == binds->prev)
        binds->command = 3;
}

// checks if data present in the dbus info struct
static int safety_check(DBus_Info info){
    if (info.artist_str && info.album_str)
        return false;
    else
        return true;
}

static void _render_section(char *text, int row_pad, int col_pad, int skip){
    int col_jump = 0;
    for (int x = 0; x < font->max_lines; x++){
        for (int y = 0; y < (int)strlen(text); y++){
            int val = text[y] - 32;
            val = val > 159 ? 31 : val; // if unkown char we set it to 31 for '?'
            val = val >= 0 ? val : 31;  // if unkown char we set it to 31 for '?'
            if (val > 0){
                mvprintw(x+row_pad+skip, col_pad+col_jump, "%s", font->c[val]->line[x]->lin);
                col_jump += strlen(font->c[val]->line[x]->lin);
            }
            else {
                col_jump += font->c[1]->max_width;
            }
        }
        col_jump = 0;
    }
}

static void _render_big_text(DBus_Info info, int rows, int cols){
    int row_pad = rows/2.5;
    int col_pad = cols/2+cols;
    int skip = 0;
    _render_section(info.album_str, row_pad, col_pad, skip);
    skip += font->max_lines;
    _render_section(info.artist_str, row_pad, col_pad, skip);
    skip += font->max_lines;
    _render_section(info.title_str, row_pad, col_pad, skip);
    skip += font->max_lines;
    if (info.playing)
        _render_section("<<  ||  >>", row_pad, col_pad, skip);
    else
        _render_section("<<  >   >>", row_pad, col_pad, skip);
}

static void _regular_render(DBus_Info info, int rows, int cols){
    int row_pad = rows/2.5;
    int col_pad = cols/2;
    mvprintw(0+row_pad, cols+col_pad, "%s", info.album_str);
    mvprintw(2+row_pad, cols+col_pad, "%s", info.artist_str);
    mvprintw(4+row_pad, cols+col_pad, "%s", info.title_str);
    mvprintw(6+row_pad, cols+col_pad, "%s", "<<");
    if (info.playing)
        mvprintw(6+row_pad, cols+4+col_pad, "%s", "||");
    else
        mvprintw(6+row_pad, cols+4+col_pad, "%s", ">");
    mvprintw(6+row_pad, cols+8+col_pad, "%s", ">>");
}

// diplays song name, album name, artist name
void display_song_metadata(DBus_Info info, bool big_text){
    int rows, cols = 0;
    ri.term_override = false;
    get_terminal_size(&rows, &cols, 2.5);
    if (safety_check(info)){ // make sure theres info in info
        mvprintw(0, 0, "No info");
        return;
    }
    // skips re-rendering if cover has already been rendered
    if (!ri.term_override){
        if (strcmp(ri.artist_str, info.artist_str) == 0 && strcmp(ri.album_str, info.album_str) == 0 && ri.playing == info.playing){
            ri.re_render = false;
            return;
        }
    }
    clear();
    if (big_text) _render_big_text(info, rows, cols);
    else _regular_render(info, rows, cols);
    alloc_mem_struct(&ri.artist_str, info.artist_str, 2);
    alloc_mem_struct(&ri.album_str, info.album_str, 2);
    ri.playing = info.playing;
    ri.re_render = true;
    refresh();
}

// if not using color then calculates the proper char that corresponds
// tho the grayscale value of the data
// compare w/ chafa
char get_ascii_value(uint8_t *data, int i, int j, int rows, int cols, int w){
    int tot = 0;
    int loops = 0;
    for (int x = i*rows; x < cols+i*rows; x++){
        for (int y = j*cols; y < rows+j*cols; y++){
            for (int z = 0; z < 3; z++){
                loops++;
                tot += data[((x*w+y)*3+z)];
            }
        }
    }
    float avg = (float)tot/(float)loops/255*8;
    return g_char[(int)avg];
}

// if using color calculates the id of the nearest color to the one in the
// album cover
int get_avg_color(uint8_t *data, int i, int j, int rows, int cols, int w){
    int r = 0, g = 0, b = 0;
    int loops = 0;
    for (int x = i*rows; x < cols+i*rows; x++){
        for (int y = j*cols; y < rows+j*cols; y++){
            loops++;
            r += data[((x*w+y)*3)];
            g += data[((x*w+y)*3+1)];
            b += data[((x*w+y)*3+2)];
        }
    }

    r /= loops;    r /= 64;
    g /= loops;    g /= 64;
    b /= loops;    b /= 64;
        
    return calc_equation(r, g, b);
}

// renders album cover
void render_album_cover(DBus_Info info, bool color){
    if (safety_check(info))
        return;
    if (!ri.re_render){ // set to true in display_song_metadata
        return; // skips re-rendering album cover if already displaying
    }
    int w, h, c = 0;
    if (!info.cover_path || strlen(info.cover_path) < 8){ // safe tea check for valid path
        return;
    }
    info.cover_path += 7; // skip the file:/// part
    uint8_t *data = stbi_load(info.cover_path, &w, &h, &c, 3);
    if (!data){
        mvprintw(0, 0, "No data! path: %s", info.cover_path);
        refresh();
        return;
    }
    int cover_w, cover_h = 0; //cover_w -> self.cols in python
    get_terminal_size(&cover_h, &cover_w, 2);
    // sometimes loads "mini" cover, so this checks for fp exceptions
    cover_w = cover_w <= w ? cover_w : w; 
    int x_scale = w/cover_w; // w in python 
    int w_scale = x_scale/0.5; // h in python
    int rows = h/w_scale; // self.rows in python
    int row_pad = cover_h/9;
    int col_pad = cover_w/9;

    int cp = w/cover_w;
    int rp = h/rows;
    render_count++;
    //mvprintw(0, 0, "Render count: %d ", render_count);
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cover_w; j++){
            if (color){
                int x = get_avg_color(data, i, j, rp, cp, w);
                attron(COLOR_PAIR(x));
                mvaddch(row_pad+i, col_pad+j, ' ');
                attroff(COLOR_PAIR(x));
            } else {
                char x = get_ascii_value(data, i, j, rp, cp, w);
                mvprintw(row_pad+i, col_pad+j,"%c", x);
            }
        }
    }
    refresh();
    free(data);
    // sets to false so no rerendering until 
    // display_song_metadata finds new data
    ri.re_render = false;
}