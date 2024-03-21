import curses
from curses import wrapper
from dbus_int import MusicManager
import pyfiglet

terminal_colors = {
    'x': 1,
    'r': 2,
    'g': 3,
    'b': 4,
    'y': 5,
    'p': 6,
    't': 7,
    'w': 8
}

keybinds = {
    'pause' : [ord('p'), ord('P')],
    'skip' : [ord('['), ord('[')],
    'prev' : [ord('o'), ord('O')],
    'quit' : [ord('q'), ord('Q')]
}

def init_colors():
    curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_RED, curses.COLOR_RED)
    curses.init_pair(3, curses.COLOR_GREEN, curses.COLOR_GREEN)
    curses.init_pair(4, curses.COLOR_BLUE, curses.COLOR_BLUE)
    curses.init_pair(5, curses.COLOR_YELLOW, curses.COLOR_YELLOW)
    curses.init_pair(6, curses.COLOR_MAGENTA, curses.COLOR_MAGENTA)
    curses.init_pair(7, curses.COLOR_CYAN, curses.COLOR_CYAN)
    curses.init_pair(8, curses.COLOR_WHITE, curses.COLOR_WHITE)

def print_debug(screen, text):
    screen.addstr(0, 0, text)

def print_figlet(stdscr, txt, ypad, xpos):
    bunk, true_i = 1, 0
    for t in txt:
        stdscr.addstr(ypad+bunk, xpos+true_i, t, curses.A_BOLD)
        true_i += 1
        if t == '\n':
            bunk += 1
            true_i = 0
    return bunk

def check_len(txt, bp):
    if len(txt) > bp:
        txt = txt[:bp-3]
        txt += "..."
    return txt

def display_with_figlet(stdscr, manager, color):
    xpad = int(manager.cols/5)
    ypad = int(manager.cols/15)
    xpos = xpad+manager.cols+xpad
    if not color:
        for i, line in enumerate(manager.cover_art):
            stdscr.addstr(ypad+i, xpad, line)
    else:
        for i, line, in enumerate(manager.cover_art):
            for i2, char, in enumerate(line):
                stdscr.addch(
                    ypad+i, 
                    xpad+i2, 
                    char, 
                    curses.color_pair(terminal_colors[char])
                )
    curses.curs_set(0)
    song = check_len(manager.song, int(manager.cols/4))
    artist = check_len(manager.artist, int(manager.cols/4))
    ablum = check_len(manager.album, int(manager.cols/4))
    ypad -= 2
    ypad += print_figlet(
        stdscr, 
        list(pyfiglet.figlet_format(
            artist,
            width=manager.get_terminal_size()*3-xpad-manager.cols-xpad)), 
        ypad, 
        xpos
    )
    ypad += print_figlet(
        stdscr, 
        list(pyfiglet.figlet_format(
            song, 
            width=manager.get_terminal_size()*3-xpad-manager.cols-xpad)), 
        ypad, 
        xpos
    )
    ypad += print_figlet(
        stdscr, 
        list(pyfiglet.figlet_format(
            ablum, 
            width=manager.get_terminal_size()*3-xpad-manager.cols-xpad)), 
        ypad, 
        xpos
    )
    print_figlet(stdscr, list(pyfiglet.figlet_format("<<")), manager.rows-2, xpos)
    if manager.playing == "Playing":
        print_figlet(stdscr, list(pyfiglet.figlet_format("||")), manager.rows-2, xpos+10)
    else:
        print_figlet(stdscr, list(pyfiglet.figlet_format(">")), manager.rows-2, xpos+10)
    print_figlet(stdscr, list(pyfiglet.figlet_format(">>")), manager.rows-2, xpos+19)
    manager.changed_state = False
    return

def display(stdscr, manager, color):
    curses.use_default_colors()
    if color:
        init_colors()
    manager.read_and_convert_cover(color=color)
    stdscr.clear()
    stdscr.nodelay(True)
    if manager.cols > 50:
        display_with_figlet(stdscr, manager, color)
        stdscr.refresh()
        return
    xpad = int(manager.cols/5)
    ypad = int(manager.cols/10)
    if not color:
        for i, line in enumerate(manager.cover_art):
            stdscr.addstr(ypad+i, xpad, line)
    else:
        for i, line, in enumerate(manager.cover_art):
            for i2, char, in enumerate(line):
                stdscr.addch(
                    ypad+i, 
                    xpad+i2, 
                    char, 
                    curses.color_pair(terminal_colors[char])
                )
    curses.curs_set(0)
    xpos = xpad+manager.cols+xpad
    stdscr.addstr(ypad+1, xpos, manager.artist)
    stdscr.addstr(ypad+3, xpos, manager.song)
    stdscr.addstr(ypad+5, xpos, manager.album)
    stdscr.addstr(ypad+manager.rows-1, xpos, "<<", curses.A_BOLD)
    if manager.playing == 'Playing':
        stdscr.addstr(ypad+manager.rows-1, xpos+4, "||", curses.A_BOLD)
    else:
        stdscr.addstr(ypad+manager.rows-1, xpos+5, ">", curses.A_BOLD)
    stdscr.addstr(ypad+manager.rows-1, xpos+8, ">>", curses.A_BOLD)
    stdscr.refresh()
    manager.changed_state = False

def music_controls(stdscr, manager, k, color):
    if k == keybinds['pause'][0] or k == keybinds['pause'][1]:
        manager.interface.PlayPause()
        display(stdscr, manager, color)
    elif k == keybinds['skip'][0] or k == keybinds['skip'][1]:
        manager.interface.Next()
    elif k == keybinds['prev'][0] or k == keybinds['skip'][1]:
        manager.interface.Previous()

def keyboard_manager(stdscr, manager, color):
    k = stdscr.getch()
    if k == keybinds['quit'][0] or k == keybinds['quit'][1]:
        return True
    if manager.interface:
        music_controls(stdscr, manager, k, color)
    return False


def main(stdscr, manager, color):
    from time import sleep
    quit = False
    while not quit:
        manager.get_music_metadata()
        if manager.redisplay or manager.changed_state:
            display(stdscr, manager, color)
        elif manager.get_terminal_size() != manager.cols:
            manager.cols = manager.get_terminal_size()
            manager.redisplay = True
            display(stdscr, manager, color)
        else:
            quit = keyboard_manager(stdscr, manager, color)

def set_new_keybinds(binds):
    for b, x in zip(binds, ['pause', 'skip', 'prev']):
        keybinds[x][0] = ord(b)
        keybinds[x][1] = ord(b.upper())

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-color', dest='color', required=False, action='store_true')
    parser.add_argument('--keybinds', dest='binds', required=False)
    args = parser.parse_args()
    if args.binds:
        set_new_keybinds(args.binds)
    args.color = not args.color
    manager = MusicManager()
    wrapper(main, manager, args.color)