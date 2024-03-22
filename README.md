# TerminalMusic
Python scripts that displays & controls the default Linux mediaplayer in the terminal. Script uses dbus to gather metadata about the music and to send control signals. Also uses ncurses to create the terminal UI.

## Libraries:
 - Internal: dbus, curses, re, os
 - External: PIL, numpy, pyfiglet

## Controls:
 - Defualt controls set to: 'p'=play/pause, 'o'=previous, '['=skip
 - This can be changed by passing a string to argument `--keybinds`
    - Example: --keybinds 'jkl' would set 'j'=pause, 'k'=skip, 'l'=previous
 - Can switch from color to ASCII mode by passing argument `--no-color`

## Visuals:
Mini text-mode with ASCII only cover (left), Color cover (right)
<div align="left">
    <img src="/screenshots/Screenshot from 2024-03-21 18-58-15.png" width="400px"</img> 
    <img src="/screenshots/Screenshot from 2024-03-21 18-58-23.png" width="400px"</img> 
</div>


Large/Figlet text-mode with ASCII only cover (left), Color cover 
<div align="left">
    <img src="/screenshots/Screenshot from 2024-03-21 18-57-23.png" width="400px"</img> 
    <img src="/screenshots/Screenshot from 2024-03-21 18-57-13.png" width="400px"</img> 
</div>


Full desktop view (yes I'm doing this to show off my rice)
<div align="left">
    <img src="/screenshots/Screenshot from 2024-03-21 18-56-26.png" width="800px"</img> 
</div>
