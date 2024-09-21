# TxM
Terminal Music.

I suffered through using the default dbus api so this thing wouldn't requrie any dependencies.

## CLI Flags:
 - `--nocolor` flag disables colors and renders covers with ascii only
 - `--font=` flag renders text with .k9 fonts (Walmart figlet)
    - Value passed in after font should be the path to the desired font
    - Will attempt to read from local directory first and then HOME directory
 - `--help` flag will print help screen and exit
 - `--debug` flag along with debug key allows access to debug screen to see program data
 - **config.cfg** file determines what keybinds to use. If deleted will be recreated

## Keyboard Binds:
Written as they are in the config file:
 - **PLAY**. Default: `p`, acts as play/pause button
 - **NEXT**. Default: `[`, skips to next track if possible.
 - **PREV**. Default: `o`, skips to previous track if possible.
 - **QUIT**. Default: `q`, acts as back and exit button
 - **SWCH**. Default: `s`, switches between players when available
 - **DEBG**. Default: `d`, when `--debug` flag is passed this will show program info

## External Tools
**fig2k9.py** is a python script that converts figlet fonts (.flf) into my fonts (.k9). 
Found it easier to do it this way.

## Install.sh
Simple bash script to install TxM. Will move binary into user's **`bin`** folder. Will also create a hidden **`.txm`** folder in user's HOME directory to hold config file along with a few of the fonts.

## Fonts
k9 fonts are simple files where `$` indicate the start of a charachter, next charachter is what the charachter is (this isn't necessary mainly for debugging). 
The charachter is then created (can't use `$` in it), and `$$` indicates that the charachter has been completed. Charchters should be defined in their ascii order
starting with space. Example of defining the 'a' charachter:

```
$a
    ___
   /   \
  / / \ \
 / /---\ \
/_/     \_\$$
```

Comments are defined with a `#` at the beginning of a line and end when the newline character is hit.
