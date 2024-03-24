import dbus
import re
import numpy as np
from PIL import Image
import os

colors = {
    "@" : '\033[1;31m',
    "%" : '\033[1;32m',
    "#" : '\033[0;101m',
    "*" : '\033[1;34m',
    "+" : '\033[1;35m',
    "=" : '\033[1;36m',
    "-" : '\033[1;37m',
    ":" : '\033[0;105m',
    "." : '\033[0;102m',
    "?" : '\033[0;103m',
    "x" : '\033[1;33m',
    " " : '\033[0;90m',
    "RESET" : '\033[0m',
}

class MusicManager:
    def __init__(self):
        self.session = None
        self.playing = None
        self.song = None
        self.album = None
        self.artist = None
        self.meta = None
        self.cover_art = None
        self.scale = ' ?@%#*+=-:.x'
        self.changed_state = True
        self.cols = self.get_terminal_size()
        self.rows = 0
        self.size = 0.5
        self.cover_file = None
        self.redisplay = False
        self.interface = None
        self.ini_player()
    
    def ts(self):
        return os.get_terminal_size().columns

    def get_terminal_size(self):
        rc = min(os.get_terminal_size().columns, os.get_terminal_size().lines*4.933)
        return int(rc/3)

    def ini_player(self):
        # thank u:
        # https://askubuntu.com/questions/1298707/command-to-get-list-of-all-media-playing
        self.session = dbus.SessionBus()

        service = [service for service in self.session.list_names() if service.startswith('org.mpris.MediaPlayer2.')]
        if not service:
            self.session = None
            return
        service = service[0]
        self.session = self.session.get_object(service, '/org/mpris/MediaPlayer2') # this
        self.playing = self.session.Get('org.mpris.MediaPlayer2.Player', 'PlaybackStatus', dbus_interface='org.freedesktop.DBus.Properties')
        self.meta = self.session.Get('org.mpris.MediaPlayer2.Player', 'Metadata', dbus_interface='org.freedesktop.DBus.Properties')

        # thank u:
        # https://stackoverflow.com/questions/12165082/play-a-musiclist-with-d-bus-in-mediaplayer
        self.interface = dbus.Interface(self.session, 'org.mpris.MediaPlayer2.Player')

    def null_load(self):
        self.playing = "Stopped"
        self.album = "Nothing playing"
        self.artist = "Nothing playing"
        self.song = "Nothing playing"
        self.cover_art = []

    def get_music_metadata(self):
        if self.session == None:
            self.null_load()
            self.ini_player()
            return
        playing = self.session.Get('org.mpris.MediaPlayer2.Player', 'PlaybackStatus', dbus_interface='org.freedesktop.DBus.Properties')
        if self.playing != playing:
            self.playing = playing
            self.changed_state = True
        self.playing = playing
        self.meta = self.session.Get('org.mpris.MediaPlayer2.Player', 'Metadata', dbus_interface='org.freedesktop.DBus.Properties')
        if self.cover_file == re.sub('file://', '', self.meta['mpris:artUrl']) or self.changed_state:
            if self.song == self.meta['xesam:title'] and self.artist == self.meta['xesam:artist'][0]:
                self.redisplay = False
                return
        self.redisplay = True
        self.song = self.meta['xesam:title']
        self.album = self.meta['xesam:album']
        self.artist = self.meta['xesam:artist'][0]

        self.cover_file = re.sub('file://', '', self.meta['mpris:artUrl'])

    def get_px_color(self, cover, x1, x2, y1, y2, t=130):
        ret_color = 'x'
        img = cover[y1: y2, x1: x2, 0]
        avg_r = int(self.getAverageL(img))
        img = cover[y1: y2, x1: x2, 1]
        avg_g = int(self.getAverageL(img))
        img = cover[y1: y2, x1: x2, 2]
        avg_b = int(self.getAverageL(img))
        if avg_r > t:
            ret_color = 'r'
        if avg_g > t:
            ret_color = 'g'
        if avg_b > t:
            ret_color = 'b'
        if avg_r > t and avg_g > t:
            ret_color = 'y'
        if avg_r > t and avg_b > t:
            ret_color = 'p'
        if avg_g > t and avg_b > t:
            ret_color = 't'
        if avg_r > t and avg_g > t and avg_b > t:
            ret_color = 'w'
        return ret_color
    

    def read_and_convert_cover(self, color = False):
        # thank u:
        # https://www.geeksforgeeks.org/converting-image-ascii-image-python/
        if not self.redisplay:
            return
        try:
            cover = np.array(Image.open(self.cover_file))#.convert('HSV'))
        except:
            self.cover_file = None
            return
        if not color:
            cover = cover[:, :, 0]
            H, W = cover.shape
        else:
            H, W, _ = cover.shape

        if self.cols > W:
            self.cols = W
        w = W/self.cols
        h = w/self.size
        rows = int(H/h)
        self.rows = rows

        self.cover_art = []
        for j in range(rows):
            y1 = int(j*h)
            y2 = int((j+1)*h)

            if j == rows-1:
                y2 = H
            
            self.cover_art.append("")
            for i in range(self.cols):
                x1 = int(i*w)
                x2 = int((i+1)*w)

                if i == self.cols-1:
                    x2 = W
                if color:
                    self.cover_art[j] += self.get_px_color(cover, x1, x2, y1, y2)
                else:
                    img = cover[y1: y2, x1: x2]
                    avg = int(self.getAverageL(img))
                    gsval = self.scale[int((avg*len(self.scale)-1)/255)]
                    self.cover_art[j] += gsval

    def getAverageL(self, img):
        w,h = img.shape
        return np.average(img.reshape(w*h))
    
    def display_all(self):
        if not self.redisplay:
            return
        os.system('clear')
        for row in self.cover_art:
            for c in row:
                print(f"{colors[c]}{c}{colors['RESET']}", end='')
            print()
        print(self.artist)
        print(self.song)
        print(self.album)
    
    def edit_song(self, song):
        self.song = song

    
if __name__ == "__main__":
    import time
    mp = MusicManager()
    mp.get_music_metadata()
    while 1:
        mp.get_music_metadata()
        mp.redisplay = True
        mp.read_and_convert_cover()
        mp.display_all()
        time.sleep(1)


