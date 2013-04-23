#!/usr/bin/env python

import sys

#temporary hack, we'll be moving to virtualenv.
sys.path.insert(1,'./gen-py')

from spotify import Spotify
from spotify.ttypes import *
from spotify.constants import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

import curses
from curses import panel

import signal
import getpass

SPOTIFYD_PORT = 9090

def signal_handler(signal, frame):
    curses.endwin()
    sys.exit(0)

class spclient(object):

    def __init__(self):
        # Make socket
        self._transport = TSocket.TSocket('localhost', SPOTIFYD_PORT)

        # Buffering is critical. Raw sockets are very slow
        self._transport = TTransport.TBufferedTransport(self._transport)

        # Wrap in a protocol
        self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)

        # Create a client to use the protocol encoder
        self._client = Spotify.Client(self._protocol)

        # Connect!
        self._transport.open()


        #status
        self._success = True

        # Empty credential
        self._credentials = None

        # Empty playlist array
        self._playlists = []

        # Empty selected playlist
        self._currentplaylist = None

        # init curses screen
        self._screen = curses.initscr()
        self._window = self._screen.subwin(0,0)
        self._menu = panel.new_panel(self._window)
        #self._menu.window().resize(16, 22);
        #self._menu.window().border(1)

        self._playlistpanel = panel.new_panel(self._window)
        self._playlistpanel.hide()
        panel.update_panels()



    def get_param(self, window, prompt_string, passwd=False, row=2, col=2):
        window.border(0)
        window.addstr(row, col, prompt_string)
        window.refresh()
        if passwd:
            curses.noecho()
        input = window.getstr(row, col+len(prompt_string)+1, 60)
        curses.echo()
        return input

    def spot_login(self):
        self._window.clear()
        username = self.get_param(self._window, "username: ")
        password = self.get_param(self._window, "password: ", True, 3, 2)

        try:
            credentials = SpotifyCredential( username, password )
            self._credentials = self._client.loginSession(credentials)
        except Exception, e:
            self._window.clear()
            return None

        return credentials

    """"
    Because libspotify is async we need this to check we logged
    in succesfully.
    """
    def spot_isloggedin(self, username=None, uid=None):
       ret = False
       if username is None and uid is None:
           return ret

       try:
            credentials = SpotifyCredential( username )
            ret = self._client.isLoggedIn(credentials)

       except Exception, e:
           self._window.addstr(30, 2, e.__str__())
           self._success = False
       finally:
           return ret

    def spot_getplaylists(self):
        try:
            #self._window.clear()
            #self._window.border(0)
            """ pls will be a set with the playlists """
            pls = self._client.getPlaylists(self._credentials)

            row = 3
            plwindow = self._playlistpanel.window()
            for p in pls:
                self._playlists.append(p)
                plwindow.addstr(row, 3, "%d. %s" % (row-2, p))
                row += 1

            plwindow.resize(20, 15)
            plwindow.move(10, 25)
            self._playlistpanel.show()
            panel.update_panels()
            curses.doupdate()
            #self._window.refresh()

        except Exception, e:
            self._window.addstr(30, 2, e.__str__())
            self._success = False
            return None


    def spot_selplaylist(self):
        try:
            self._window.clear()
            self._window.border(0)
            row = 5
            for p in self._playlists:
                self._window.addstr(row, 3, "%d. %s" % (row-4, p))
                row += 1
            plidx = self.get_param("Playlist to select: ")
            self._client.selectPlaylist(self._credentials, self._playlists[int(plidx)])

        except Exception, e:
            self._window.addstr(30, 2, e.__str__())
            self._success = False
            return None

    def spot_seltrack(self):
        return None

    def spot_playback(self):
        try:
            #keeping it real simple for now.
            self._client.sendCommand(self._credentials, SpotifyCmd.PLAY)

        except Exception, e:
            self._window.addstr(30, 2, e.__str__())
            self._success = False
            return None

    def spot_getcurrent(self):
        return None

    def spot_logout(self):
        return None


class Menu(object):
    def __init__(self, items, stdscreen):
        self.window = stdscreen.subwin(0,0)
        self.window.keypad(1)
        self.panel = panel.new_panel(self.window)
        self.panel.hide()
        panel.update_panels()

        self.position = 0
        self.items = items
        self.items.append(('Done', 'Done'))

    def navigate(self, n):
        self.position += n
        if self.position < 0:
            self.position = 0
        elif self.position >= len(self.items):
            self.position = len(self.items)-1

    def display(self):
        self.panel.top()
        self.panel.show()
        self.window.clear()

        while True:
            self.window.refresh()
            curses.doupdate()
            for index, item in enumerate(self.items):
                if index == self.position:
                    mode = curses.A_REVERSE
                else:
                    mode = curses.A_NORMAL

                msg = '%d. %s' % (index, item[0])
                self.window.addstr(1+index, 1, msg, mode)

            key = self.window.getch()

            if key in [curses.KEY_ENTER, ord('\n')]:
                if self.position == len(self.items)-1:
                    break
                else:
                    self.items[self.position][1]()

            elif key == curses.KEY_UP:
                self.navigate(-1)

            elif key == curses.KEY_DOWN:
                self.navigate(1)


        self.window.clear()
        self.panel.hide()
        panel.update_panels()
        curses.doupdate()


class FieldMenu(Menu):

    def display(self):
        self.panel.top()
        self.panel.show()
        self.window.clear()

        while True:
            self.window.refresh()
            curses.doupdate()
            for index, item in enumerate(self.items):
                if index == self.position:
                    mode = curses.A_REVERSE
                else:
                    mode = curses.A_NORMAL

                msg =''
                if index == len(self.items)-1:
                    msg = '%d. %s' % (index, item[0])
                else:
                    if item[2]:
                        conceal = ''
                        for c in item[1]:
                            conceal += '*'
                        msg = '%s: %s' % (item[0], conceal)
                    else:
                        msg = '%s: %s' % (item[0], item[1])

                self.window.addstr(1+index, 1, msg, mode)

            key = self.window.getch()

            if key in [curses.KEY_ENTER, ord('\n')]:
                if self.position == len(self.items)-1:
                    break

            elif key == curses.KEY_UP:
                self.navigate(-1)

            elif key == curses.KEY_DOWN:
                self.navigate(1)

            #construct new tuple. Tuples are inmutable.
            elif key == curses.KEY_BACKSPACE or key == 127:
                new_item = ( 
                        self.items[self.position][0],
                        self.items[self.position][1][:-1],
                        self.items[self.position][2]
                        )
                self.items[self.position] = new_item;

            elif key < 256:
                new_item = ( 
                        self.items[self.position][0],
                        self.items[self.position][1]+chr(key),
                        self.items[self.position][2]
                        )
                self.items[self.position] = new_item;

        self.window.clear()
        self.panel.hide()
        panel.update_panels()
        curses.doupdate()



class XplodifyApp(object):

    def __init__(self, stdscreen):
        self.screen = stdscreen
        curses.curs_set(0)

        #spoticlient = spclient()

        playback_items = [
                ('play', curses.beep),
                ('pause', curses.flash),
                ('previous', curses.flash),
                ('next', curses.flash),
                ('mode', curses.flash),
                ]
        playback = Menu(playback_items, self.screen)

        login_items = [
                ('email', '', False),
                ('password', '', True)
                ]
        login = FieldMenu(login_items, self.screen)

        main_menu_items = [
                ('login', login.display),
                ('Display Playlists', curses.flash),
                ('Select Playlists', curses.flash),
                ('Toggle Tracks', curses.flash),
                ('Select Track', curses.flash),
                ('Playback', playback.display),
                ('Logout', curses.flash),
                ]
        main_menu = Menu(main_menu_items, self.screen)
        main_menu.display()


def main():

    curses.wrapper(XplodifyApp)

    """
    SAMPLE THRIFT CALL CODE

    client.ping()
    print "ping()"

    msg = client.sayHello()
    print msg
    msg = client.sayMsg(HELLO_IN_KOREAN)
    print msg

    """

if  __name__ =='__main__':
    main()
