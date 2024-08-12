CC=gcc
LINK= -ldbus-1 -ldbus-glib-1 -lncurses
CFLAGS= -Wall -Wextra -g -lm
INCLUDE=-I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include  -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/

output: ncurses_display.o dbus_handler.o keyboard.o text.o compile clean

compile:
	$(CC) main.c dbus_handler.o ncurses_display.o keyboard.o text.o $(INCLUDE) $(LINK) $(CFLAGS) -o test

dbus_handler.o:
	$(CC) -c dbus_parser/dbus_parser.c $(INCLUDE) $(LINK) $(CFLAGS) -o $@

ncurses_display.o:
	$(CC) -c ncurses_display/ncurses_display.c $(INCLUDE) $(LINK) $(CFLAGS) -o $@

keyboard.o:
	$(CC) -c keyboard/keyboard_man.c $(INCLUDE) $(LINK) $(CFLAGS) -o $@

text.o:
	$(CC) -c text_render/text.c $(INCLUDE) $(LINK) $(CFLAGS) -o $@

clean:
	rm *.o