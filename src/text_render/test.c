#include "text.h"

int main(int argc, char *argv[]){
    if (argc < 3) return 0;
    Terminal_Font *my_font = load_font(argv[2]);
    render_text(argv[1], my_font);
    return 0;
}