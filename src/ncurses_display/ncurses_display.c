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
struct winsize w;

// allocates memory for our render info data
// if type 1 means data unallocated have to initialize
// if type 2 means data allocated just have to resize
static void alloc_mem_struct(char **str, char *string, int type){
    if (type == 1)
        *str = (char *)malloc(strlen(string)+1);
    else if (type == 2)
        *str = (char *)realloc(*str, strlen(string)+1);
    else
        exit(0);
    strcpy(*str, string);
}

static int _get_target_size(void){
    ioctl(0, TIOCGWINSZ, &w);
    if (ri.term_siz[0] != w.ws_row || ri.term_siz[1] != w.ws_col){
        // https://man7.org/linux/man-pages/man3/resizeterm.3x.html
        // force resize or it cuts text off
        resizeterm(w.ws_row, w.ws_col);
        ri.term_override = true;
    }
    ri.term_siz[0] = w.ws_row;
    ri.term_siz[1] = w.ws_col;
    float aspect_ratio = w.ws_col/(float)w.ws_row;
    return (w.ws_row*aspect_ratio > w.ws_col ? w.ws_col : w.ws_row*aspect_ratio)/2.5;
}

// uses ioctl to get size of terminal in amount of rows & cols
// uses divisor for formatting
static void get_terminal_size(int *rows, int *cols, float divisor){
    ioctl(0, TIOCGWINSZ, &w);
    if (ri.term_siz[0] != w.ws_row || ri.term_siz[1] != w.ws_col){
        resizeterm(w.ws_row, w.ws_col);
        ri.term_override = true;
    }
    *rows = w.ws_row/divisor;
    *cols = w.ws_col/divisor;
    ri.term_siz[0] = w.ws_row;
    ri.term_siz[1] = w.ws_col;
}

// initializes the color sqaures for album covers using the basic
// ncurses color pallet
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
void init_screen(bool *color, char *font_path){
    if (font_path[0] != '\n') font = load_font(font_path);
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
    // safe tea memory allocations init the char* memory slot
    alloc_mem_struct(&ri.album_str, "None", 1);
    alloc_mem_struct(&ri.artist_str, "None", 1);
    ri.playing = 2;
    ri.force_refresh = false;
    curs_set(0);
    timeout(1);
}

// if q then quit, later will include keybinds
void handle_inputs(KeyBinds *binds){
    int ch = getch();
    if (ch == binds->quit){
        binds->exit = true;
        ri.force_refresh = true;
    }
    if (ch == binds->playpause)
        binds->command = 1;
    if (ch == binds->next)
        binds->command = 2;
    if (ch == binds->prev)
        binds->command = 3;
    if (ch == binds->debug){
        ri.force_refresh = true;
        binds->command = 4;
    }
    if (ch == binds->swch){
        ri.force_refresh = true;
        binds->command = 5;
    }
}

// checks if data present in the dbus info struct
static int safety_check(DBus_Info info){
    if (info.artist_str && info.album_str)
        return false;
    else
        return true;
}

static int _render_section(char *text, int row_pad, int col_pad, int skip){
    int col_jump = 0;
    for (int x = 0; x < font->max_lines; x++){
        for (int y = 0; y < (int)strlen(text); y++){
            int val = text[y] - 32;
            val = val > 159 ? 31 : val; // if unkown char we set it to 31 for '?'
            val = val >= 0 ? val : 31;  // if unkown char we set it to 31 for '?'
            if (val > 0){
                if (col_pad+col_jump+(int)strlen(font->c[val]->line[x]->lin) > ri.term_siz[1]){ 
                    col_jump += strlen(font->c[val]->line[x]->lin);
                    continue;
                }
                mvprintw(x+row_pad+skip, col_pad+col_jump, "%s", font->c[val]->line[x]->lin);
                col_jump += strlen(font->c[val]->line[x]->lin);
            }
            else {
                col_jump += font->c[1]->max_width;
            }
        }
        col_jump = 0;
    }
    return font->max_lines;
}

static void _render_big_text(DBus_Info info, int target_cols, int cols){
    int row_pad = target_cols/8;
    int col_pad = row_pad + target_cols + row_pad;
    row_pad /= 2;
    int skip = 0;
    skip += _render_section(info.title_str, row_pad, col_pad, skip);
    skip += _render_section(info.artist_str, row_pad, col_pad, skip);
    skip += _render_section(info.album_str, row_pad, col_pad, skip);
    if (row_pad+ri.img_size-font->max_lines > row_pad+skip)
        skip = ri.img_size-font->max_lines;
    if (info.playing){
        (void)_render_section("<<", row_pad, col_pad, skip);
        (void)_render_section("||", row_pad, (col_pad+cols-10)/2, skip);
        (void)_render_section(">>", row_pad, cols-10, skip);
    } else {
        (void)_render_section("<<", row_pad, col_pad, skip);
        (void)_render_section(">", row_pad, (col_pad+cols-10)/2, skip);
        (void)_render_section(">>", row_pad, cols-10, skip);
    }
}

static bool _check_regular_overflow(char *compare_str, int col_pad){
    return ((int)strlen(compare_str)+col_pad > ri.term_siz[1]);
}

static void _regular_render(DBus_Info info, int target_cols, int cols){
    int row_pad = target_cols/8;
    int col_pad = row_pad + target_cols + row_pad;
    row_pad /= 2;
    char *tmp_str = (char *)malloc(1);
    if (_check_regular_overflow(info.title_str, col_pad)){
        tmp_str = realloc(tmp_str, ri.term_siz[1]-col_pad);
        strncpy(tmp_str, info.title_str, ri.term_siz[1]-col_pad-2);
        tmp_str[ri.term_siz[1]-col_pad-2] = '\0';
        mvprintw(0+row_pad, col_pad, "%s", tmp_str);
    } else {
        mvprintw(0+row_pad, col_pad, "%s", info.title_str);
    }
    if (_check_regular_overflow(info.artist_str, col_pad)){
        tmp_str = realloc(tmp_str, ri.term_siz[1]-col_pad);
        strncpy(tmp_str, info.artist_str, ri.term_siz[1]-col_pad-2);
        tmp_str[ri.term_siz[1]-col_pad-2] = '\0';
        mvprintw(2+row_pad, col_pad, "%s", tmp_str);
    } else {
        mvprintw(2+row_pad, col_pad, "%s", info.artist_str);
    }
    if (_check_regular_overflow(info.album_str, col_pad)){
        tmp_str = realloc(tmp_str, ri.term_siz[1]-col_pad);
        strncpy(tmp_str, info.album_str, ri.term_siz[1]-col_pad-2);
        tmp_str[ri.term_siz[1]-col_pad-2] = '\0';
        mvprintw(4+row_pad, col_pad, "%s", tmp_str);
    } else {
        mvprintw(4+row_pad, col_pad, "%s", info.album_str);
    }
    if (ri.img_size+row_pad-2 > 4+row_pad)
        row_pad = ri.img_size+row_pad-1;
    else
        row_pad = row_pad + 6;
    free(tmp_str);
    mvprintw(row_pad, col_pad, "%s", "<<");
    if (info.playing)
        mvprintw(row_pad, (cols-3+col_pad)/2, "%s", "||");
    else
        mvprintw(row_pad, (cols-3+col_pad)/2, "%s", ">");
    mvprintw(row_pad, cols-3, "%s", ">>");
}

// diplays song name, album name, artist name
void display_song_metadata(DBus_Info info, char *font_path){
    int target_cols = _get_target_size();
    int rows, cols = 0;
    get_terminal_size(&rows, &cols, 1);
    if (!ri.re_render){ // set to true in render_album_cover
        return; // skips re-rendering already displaying
    }
    //mvprintw(0, 0, "Term size: %d %d", rows, cols);
    if (font_path[0] != '\n') _render_big_text(info, target_cols, cols);
    else _regular_render(info, target_cols, cols);
    alloc_mem_struct(&ri.artist_str, info.artist_str, 2);
    alloc_mem_struct(&ri.album_str, info.album_str, 2);
    ri.playing = info.playing;
    ri.re_render = false;
    refresh();
}

// if not using color then calculates the proper char that corresponds
// tho the grayscale value of the data
char get_ascii_value(uint8_t *data, int x1, int x2, int y1, int y2, int w){
    int tot = 0;
    int loops = 0;
    for (int x = x1; x < x2; x++){
        for (int y = y1; y < y2; y++){
            for (int z = 0; z < 3; z++){
                loops++;
                tot += data[((y*w+x)*3+z)];
            }
        }
    }
    float avg = (float)tot/(float)loops/255*8;
    return g_char[(int)avg];
}

// if using color calculates the id of the nearest color to the one in the
// album cover
int get_avg_color(uint8_t *data, int x1, int x2, int y1, int y2, int w){
    int r = 0, g = 0, b = 0;
    int loops = 0;
    for (int x = x1; x < x2; x++){
        for (int y = y1; y < y2; y++){
            loops++;
            r += data[((y*w+x)*3)];
            g += data[((y*w+x)*3+1)];
            b += data[((y*w+x)*3+2)];
        }
    }

    r /= loops;    r /= 64;
    g /= loops;    g /= 64;
    b /= loops;    b /= 64;
    
    return calc_equation(r, g, b);
}

// renders album cover
void render_album_cover(DBus_Info info, bool color){
    if (safety_check(info)){ // make sure theres info in dbus info struct
        mvprintw(0, 0, "No info");
        return;
    }
    ri.term_override = false; 
    int target_cols = _get_target_size(); // doing this here to get override variable
    // skips re-rendering if cover has already been rendered
    if (!ri.term_override && !ri.force_refresh){
        if (strcmp(ri.artist_str, info.artist_str) == 0 && 
            strcmp(ri.album_str, info.album_str) == 0 && 
            ri.playing == info.playing){
            ri.re_render = false;
            return;
        }
    }
    if (!info.cover_path || strlen(info.cover_path) < 8){ // safe tea check for valid path
        mvprintw(0, 0, "No cover path found!\n");
        return;
    }
    // Begin rendering
    ri.re_render = true;
    clear();
    render_count++;
    //mvprintw(1, 0, "Render count: %d ", render_count);
    int img_w, img_h, img_c = 0;
    // if its a file:// path then we want to skip the file:// part
    if (strncmp(info.cover_path, "file://", strlen("file://")) == 0)
        info.cover_path += 7;
    uint8_t *data = stbi_load(info.cover_path, &img_w, &img_h, &img_c, 3);
    if (!data){
        mvprintw(0, 0, "No data! path: %s", info.cover_path);
        // forces screen refresh until cover is renderable
        ri.force_refresh = true;
        refresh();
        return;
    }
    // size limitation
    if (target_cols > img_w) target_cols = img_w;
    float render_w = img_w/target_cols;
    float render_h = render_w/0.5; // self.size
    ri.img_size = img_h/render_h;
    int x1, x2, y1, y2 = 0;
    for (int i = 0; i < ri.img_size; i++){
        y1 = i*render_h;
        y2 = (i+1)*render_h;
        if (i == ri.img_size-1) y2 = img_h;
        for (int j = 0; j < target_cols; j++){
            x1 = j*render_w;
            x2 = (j+1)*render_w;
            if (j == target_cols-1) x2 = img_w;
            if (color){
                int x = get_avg_color(data, x1, x2, y1, y2, img_w);
                attron(COLOR_PAIR(x));
                mvaddch(target_cols/16+i, target_cols/8+j, ' ');
                attroff(COLOR_PAIR(x));
            } else {
                char x = get_ascii_value(data, x1, x2, y1, y2, img_w);
                mvprintw(target_cols/16+i, target_cols/8+j, "%c", x);
            }
        }
    }
    free(data);
    ri.force_refresh = false;
}

char *_clean_if_name(char *interface){
    char *name = (char *)malloc(strlen(interface)+1);
    strcpy(name, interface);
    for (int i = 0; i < 23; i++){
        for (int j = 1; j < (int)strlen(interface)+1; j++)
            name[j-1] = name[j];
    }
    name = realloc(name, strlen(interface)-22);
    int new_len = 0;
    for (int x = 0; x < (int)strlen(name); x++){
        if (name[x] == '.') break;
        new_len += 1;
    }
    name[new_len] = 0;
    name = realloc(name, new_len);
    return name;
}

void render_dbus_sources(DBus_Info info, KeyBinds binds){
    if (!ri.force_refresh)
        return;
    clear();
    char *if_name = _clean_if_name(info.player_interface);
    mvprintw(0, 0, "DBUS info:");
    mvprintw(1, 0, "\tSource:\t\t%s", if_name);
    mvprintw(2, 0, "\tSource index:\t%d", info.if_curr_idx);
    mvprintw(3, 0, "\tCover path:\t%s", info.cover_path);
    mvprintw(4, 0, "\tAlbum:\t\t%s", info.album_str);
    mvprintw(5, 0, "\tArtist\t\t%s", info.artist_str);
    mvprintw(6, 0, "\tSong:\t\t%s", info.title_str);
    mvprintw(7, 0, "\tPlaying:\t%d", info.playing);
    mvprintw(8, 0, "\tAll sources:");
    int i = 0;
    for (; i < info.if_list_len; i++){
        mvprintw(2*i+1+8, 0, "\t\tInterface name:\t%s", info.if_list[i]->name);
        mvprintw(2*i+2+8, 0, "\t\tInterface id:\t%s", info.if_list[i]->id);
    }
    i+=2;
    mvprintw(9+i, 0, "Render info:");
    mvprintw(10+i, 0, "\tTerm size:  %d rows, %d columns", ri.term_siz[0], ri.term_siz[1]);
    mvprintw(11+i, 0, "\tImage size: %d", ri.img_size);
    i += 1;
    mvprintw(12+i, 0, "Keybind info:");
    mvprintw(13+i, 0, "\tQuit:   %c", binds.quit);
    mvprintw(14+i, 0, "\tPlay:   %c", binds.playpause);
    mvprintw(15+i, 0, "\tNext:   %c\tPrev:   %c", binds.next, binds.prev);
    mvprintw(16+i, 0, "\tSwitch: %c\tDebug:  %c", binds.swch, binds.debug);
    refresh();
    free(if_name);
    ri.force_refresh = false;
}