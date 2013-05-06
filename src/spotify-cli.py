#!/usr/bin/env python

import sys
import time

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

import urwid

import signal
import getpass

SPOTIFYD_PORT = 9090
VERSION = "0.1"

def signal_handler(signal, frame):
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

    def login(self, username, password):
        success=False
        try:
            credentials = SpotifyCredential( username, password )
            self._credentials = self._client.loginSession(credentials)
            success=True #refine this.
        except Exception, e:
            return False

        return success

    def logout(self):
        try:
            if self._credentials:
                self._client.logoutSession(self._credentials)
                self._credentials = None
        except Exception, e:
            return False

        return True

    """"
    Because libspotify is async we need this to check we logged
    in succesfully.
    """
    def loggedin(self, username=None, uid=None):
       ret = False
       if username is None and uid is None:
           return ret

       try:
            credentials = SpotifyCredential( username )
            ret = self._client.isLoggedIn(credentials)

       except Exception, e:
           self._success = False
       finally:
           return ret

    def getplaylists(self):
        pls = None
        try:
            """ pls will be a set with the playlists """
            pls = self._client.getPlaylists(self._credentials)

        except Exception, e:
            self._success = False

        return pls

    def spot_selplaylist(self):
        try:
            self._window.clear()
            self._window.border(0)
            row = 5
            for p in self._playlists:
                row += 1
            plidx = self.get_param("Playlist to select: ")
            self._client.selectPlaylist(self._credentials, self._playlists[int(plidx)])

        except Exception, e:
            self._success = False
            return None

    def spot_seltrack(self):
        return None

    def spot_playback(self):
        try:
            #keeping it real simple for now.
            self._client.sendCommand(self._credentials, SpotifyCmd.PLAY)

        except Exception, e:
            self._success = False
            return None

    def spot_getcurrent(self):
        return None

    def spot_logout(self):
        return None



class XplodifyElement(urwid.Button):
    def __init__(self, el_id, el_name, callback=None, userdata=None):
        self.el_id = el_id 
        self.el_name = el_name
        super(XplodifyElement, self).__init__(u"" + self.el_name, on_press=callback, user_data=userdata)


class XplodifyDisplay(urwid.Frame):
    palette = [
            ('body','default', 'default'),
            ('foot','black', 'light gray', 'bold'),
            ('key','light cyan', 'dark blue', 'underline'),
            ]

    footer_text = ('foot', [
        "Xpldofiy Client "+ VERSION +" -     " ,
        ('key', "F4"), " login   ",
        ('key', "F5"), " |<   ",
        ('key', "F7"), " |> / ||   ",
        ('key', "F8"), " >|   ",
        ('key', "F9"), " logout  ",
        ('key', "F10"), " quit ",
        ])

    def __init__(self):
        self.logged = False
        self.spoticlient = spclient()

        self._playlists = []
        self._plwalker = urwid.SimpleFocusListWalker([urwid.Button("(empty)")])
        self._tracks = []
        self._trwalker = urwid.SimpleFocusListWalker([urwid.Button("(empty)")])
        self.plpane = urwid.ListBox(self._plwalker)
        self.trackpane = urwid.ListBox(self._trwalker)
        self.footer = urwid.AttrWrap(urwid.Text(self.footer_text), "foot")

        self.widgets = [
                self.plpane,
                self.trackpane
                ]

        email = urwid.Edit(u'Username:  ', u"", allow_tab=False, multiline=False)
        passwd = urwid.Edit(u'Password:  ', u"", allow_tab=False, multiline=False, mask=u"*" )
        logbutton = urwid.Button(u'Login')
        urwid.connect_signal(logbutton, 'click', self.login)

        self.mainview = urwid.Columns(self.widgets, dividechars=1, focus_column=0)
        self.loginview = urwid.Filler(urwid.Pile([email, passwd,
                urwid.AttrMap(logbutton, None, focus_map='reversed')]))
        self.overlay = urwid.Overlay(self.loginview, self.mainview,
                align='center', width=('relative', 30),
                valign='middle', height=('relative', 30),
                min_width=30, min_height=6)
        super(XplodifyDisplay, self).__init__(urwid.AttrWrap(self.mainview, 'body'), footer=self.footer)

        self.loop = urwid.MainLoop(self, self.palette,
             unhandled_input=self.unhandled_keypress)

    def main(self):
        self.loop.run()

    def login_overlay(self):
        display = None
        if self.logged:
            pass
        else:
            self.body = self.overlay

    def login(self, key):
        username = self.loginview.original_widget.widget_list[0].get_edit_text()
        passwd = self.loginview.original_widget.widget_list[1].get_edit_text()
        if not self.logged:
            self.logged = self.spoticlient.login(username, passwd)
        time.sleep(1)
        if self.logged:
            self.get_playlists()
            self.mainview.set_focus_column(0)
        self.body = self.mainview

    def logout(self):
        if self.logged:
            self.spoticlient.logout()

        return

    def get_playlists(self):
        pl_set = self.spoticlient.getplaylists()
        pl_widgets = []
        if pl_set:
            pid = 1
            self._playlists = []
            for pl in pl_set:
                self._playlists.append(pl)
#                pl_widgets.append(XplodifyElement(pid, pl))
#                pl_widgets.append(button)
#                urwid.connect_signal(button, 'click', self.set_playlist)
#                pl_widgets.append(urwid.AttrMap(button, None, focus_map='reversed'))
                self._plwalker.insert(0, XplodifyElement(pid,pl))
                pid += 1

#        self.plpane = urwid.ListBox(urwid.SimpleFocusListWalker(pl_widgets))
#        self.mainview.contents[0] = (self.plpane, self.mainview.options())

    def set_playlist(self, button, playlist): 
        return

    def quit(self):
        raise urwid.ExitMainLoop()

    def unhandled_keypress(self, k):

        if k == "f4":
            self.login_overlay()
        elif k == "f5":
            raise urwid.ExitMainLoop()
        elif k == "f6":
            raise urwid.ExitMainLoop()
        elif k == "f7":
            raise urwid.ExitMainLoop()
        elif k == "f8":
            raise urwid.ExitMainLoop()
        elif k == "f9":
            raise urwid.ExitMainLoop()
        elif k == "f10":
            raise urwid.ExitMainLoop()
        else:
            return

        return True


"""
class XplodifyApp(object):

    def __init__(self, stdscreen):
        self.screen = stdscreen
        self.screen_size = self.screen.getmaxyx()
        curses.curs_set(0)
        self.track_window = self.screen.subwin(11, 50)
        self.track_window.border(0)
        self.pl_window = self.screen.subwin(self.screen_size[0]-11, 40, 11, 4)
        self.pl_window.border(0)
        self.menu_window = self.screen.subwin(10, 30, 1, 4)
        self.menu_window.border(0)

        playlist_panel = Menu([], self.pl_window, False)
        track_panel = Menu([], self.pl_window, False)

        self.xpwrap = XplodifyWrap(self.menu_window, playlist_panel, track_panel)

        playback_items = [
                ('play', curses.beep),
                ('pause', curses.flash),
                ('previous', curses.flash),
                ('next', curses.flash),
                ('mode', curses.flash),
                ]
        playback = Menu(playback_items, self.menu_window, True)

        login_items = [
                ('username', '', False),
                ('password', '', True)
                ]
        login = FieldMenu(login_items, self.menu_window, self.xpwrap.login)

        main_menu_items = [
                ('login', login.display),
                ('Display Playlists', curses.flash),
                ('Select Playlists', curses.flash),
                ('Toggle Tracks', curses.flash),
                ('Select Track', curses.flash),
                ('Playback', curses.flash),
                ('Logout', curses.flash),
                ]
        main_menu = Menu(main_menu_items, self.menu_window, True)
        main_menu.display()
"""
def main():

    XplodifyDisplay().main()

    """
    curses.wrapper(XplodifyApp)

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
