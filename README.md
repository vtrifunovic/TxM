# TxM
C version of Terminal Music

I suffered through using the default dbus api so this thing wouldn't requrie any external libraries.

## Controls:
 - passing `--nocolor` flag disables colors and renders covers with ascii only
 - passing `--font=` flag renders text with .k9 fonts (Walmart figlet)
    - Value passed in after font should be the path to the desired font
    - Will attempt to read from local directory first and then HOME directory
 - **config.cfg** file determines what keybinds to use. If deleted will be recreated

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
