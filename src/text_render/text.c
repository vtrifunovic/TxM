#include "text.h"


static void _return_line(Terminal_Char *c, FILE *font, char f){
    Line *l = (Line *)malloc(sizeof(Line));
    l->len = 0;
    l->lin = (char *)malloc(2);
    while ((f=getc(font)) != '$'){
        if (f == '\n') break;
        if (f == EOF) FATAL_ERROR("Reached EOF in unexpacted area");
        l->lin = realloc(l->lin, 3+l->len++);
        l->lin[l->len-1] = f;
    }
    l->lin[l->len] = '\0';
    c->line = realloc(c->line, sizeof(Line)*(c->cnt_lines+1));
    c->line[c->cnt_lines] = l;
    c->cnt_lines++;
}

static void _load_char(Terminal_Font *fnt, FILE *font){
    fnt->cnt_chars++;
    Terminal_Char *ch = (Terminal_Char *)malloc(sizeof(Terminal_Char));
    char f;
    ch->val = getc(font);
    ch->cnt_lines = 0;
    ch->max_width = 0;
    ch->line = (Line **)malloc(sizeof(Line)*ch->cnt_lines+1);
    (void)getc(font); // casting the \n to void
    int idx = 0;
    while ((f=getc(font)) != '$'){
        if (f == EOF){
            FATAL_ERROR("No \e[1;32m$\e[0m terminator at end of file");
        }
        _return_line(ch, font, f);
        ch->max_width = ch->line[idx]->len > ch->max_width ? ch->line[idx]->len : ch->max_width;
        idx++;
    }
    fnt->c = realloc(fnt->c, sizeof(Terminal_Char)*(fnt->cnt_chars));
    fnt->c[fnt->cnt_chars-1] = ch;
}

Terminal_Font *load_font(char *path){
    Terminal_Font *f = (Terminal_Font *)malloc(sizeof(Terminal_Font));
    f->name = (char *)malloc(strlen(path));
    f->c = (Terminal_Char **)malloc(sizeof(Terminal_Char)*1);
    f->max_lines = 0;
    f->cnt_chars = 0;
    strcpy(f->name, path);
    FILE *font = fopen(path, "r");
    if (!font){
        FATAL_ERROR("Font not found");
    }
    char ch;
    int chars_loaded = 0;
    while ((ch=getc(font)) != EOF){
        if (ch == '$'){
            _load_char(f, font);
            f->max_lines = f->c[chars_loaded]->cnt_lines > f->max_lines ? f->c[chars_loaded]->cnt_lines : f->max_lines;
            chars_loaded++;
        } else if (ch == '#'){
            // ignore comment lines
            while ((ch=getc(font)) != EOF){
                if (ch=='\n') break;
            }
        }
    }
    return f;
}

void render_text(char *text, Terminal_Font *font){
    for (int j = 0; j < font->max_lines; j++){
        for (int t = 0; t < (int)strlen(text); t++){
            int val = text[t] - 32;
            // unsupported chars get casted to 31 or '?'
            val = val > 159 ? 63 : val;
            val = val >= 0 ? val : 0;
            if (font->c[val]->cnt_lines <= j){
                for (int x = 0; x < font->c[val]->max_width; x++){
                    printf(" ");
                }
                continue;
            }
            printf("%s", font->c[val]->line[j]->lin);
        }
        puts("");
    }
}