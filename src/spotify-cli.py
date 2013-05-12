#!/usr/bin/env python

import sys
import time
import getopt
import os.path

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

import logging

XPLODIFYD_PORT = 9090
VERSION = "0.1"

logging.basicConfig(filename='debug.log',level=logging.DEBUG)

def signal_handler(signal, frame):
    sys.exit(0)

class spclient(object):

    def __init__(self, host, port):
        # Make socket
        self._transport = TSocket.TSocket(host, port)

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
            logging.debug("Exception: %s", e)
            return False

        return success

    def logout(self):
        try:
            if self._credentials:
                self._client.logoutSession(self._credentials)
                self._credentials = None
        except Exception, e:
            logging.debug("Exception: %s", e)
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
            logging.debug("Exception: %s", e)
            self._success = False
        finally:
            return ret

    def getplaylists(self):
        pls = None
        try:
            """ pls will be a set with the playlists """
            pls = self._client.getPlaylists(self._credentials)

        except Exception, e:
            logging.debug("Exception: %s", e)
            self._success = False

        return pls

    def gettracks(self, playlist):
        tracks = None
        try:
            """ pls will be a set with the playlists """
            tracks = self._client.getPlaylistByName(self._credentials, playlist)

        except Exception, e:
            logging.debug("Exception: %s", e)
            self._success = False

        return tracks

    def spot_selplaylist(self, idx):
        try:
            self._client.selectPlaylist(self._credentials, self._playlists[idx])
            self._success = True

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
            logging.debug("Exception: %s", e)
            return None

    def spot_getcurrent(self):
        return None


class XplodifyElement(urwid.Button):
    def __init__(self, el_id, el_name, callback=None, userdata=None):
        self.el_id = el_id 
        self.el_name = el_name
        super(XplodifyElement, self).__init__(u"" + self.el_name, on_press=callback, user_data=userdata)


class XplodifyApp(urwid.Frame):
    palette = [
            ('body','default', 'default'),
            ('foot','black', 'light gray', 'bold'),
            ('head','black', 'light gray', 'bold'),
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

    def __init__(self, host='localhost', port=XPLODIFYD_PORT):
        self.logged = False
        self.spoticlient = spclient(host, port)

        self._playlists = None
        self._plwalker = urwid.SimpleFocusListWalker([urwid.Button("(empty)")])
        self._tracks = {}
        self._trwalker = urwid.SimpleFocusListWalker([urwid.Button("(empty)")])
        self.plpane = urwid.ListBox(self._plwalker)
        self.trackpane = urwid.ListBox(self._trwalker)
        self.header = urwid.AttrWrap(urwid.Text(u"Not Logged In."), "head")
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
        super(XplodifyApp, self).__init__(urwid.AttrWrap(self.mainview, 'body'), footer=self.footer, header=self.header)

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
        time.sleep(2)
        if self.logged:
            self.header.original_widget.set_text(u"Logged in as "+username) 
            self.get_playlists()
            self.mainview.set_focus_column(0)
        self.body = self.mainview


    def get_playlists(self):
        try:
            self._playlists = list(self.spoticlient.getplaylists())
        except Exception, e:
            self._playlists = []
            logging.debug("Exception: %s", e)

        if self._playlists:
            #empty the playlist listwalker first...
            while self._plwalker:
                self._plwalker.pop()

            pid = 1
            for pl in self._playlists:
                self._plwalker.insert(0, XplodifyElement(pid, pl, callback=self.set_track_panel, userdata=pl))
                self.get_tracks(pl)
                pid += 1

            self.set_track_panel(None, self._playlists[0])

    def get_tracks(self, playlist):
        try:
            self._tracks[playlist] = self.spoticlient.gettracks(playlist)
        except Exception, e:
            logging.debug("Exception: %s", e)

    def set_track_panel(self, button, playlist):
        if playlist not in self._tracks:
            self.get_tracks(playlist)

        if self._tracks[playlist]:
            #empty the track listwalker first...
            while self._trwalker:
                self._trwalker.pop()

            tid=1
            for track in self._tracks[playlist]:
                logging.debug("Processing track: %s", track._name)
                self._trwalker.insert(0, XplodifyElement(track._id, track._name.decode('utf8')+" - "+track._artist.decode('utf8')))

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
            if self.logged:
                self.spoticlient.logout()
                self.header.original_widget.set_text(u"Not Logged in.")
        elif k == "f10":
            if self.logged:
                self.spoticlient.logout()
            raise urwid.ExitMainLoop()
        elif k == "tab":
            self.mainview.focus_position = (self.mainview.focus_position + 1) % 2
        else:
            return

        return True

def usage():
    BOLD_START="\033[1m"
    BOLD_END="\033[0m"
    print "usage: "+os.path.basename(__file__)+" [OPTIONS]"
    print "\nOPTIONS:"
    print "\t"+BOLD_START+"-h"+BOLD_END+"\n\t\tDisplay this help message."
    print "\t"+BOLD_START+"-s host,--server=host"+BOLD_END+"\n\t\tSpecify service hostname."
    print "\t"+BOLD_START+"-p PORT,--port=PORT"+BOLD_END+"\n\t\tSpecify service port."

def main(argv):
    server='localhost'
    port=XPLODIFYD_PORT

    try:
        opts, args = getopt.getopt(argv,"hs:p:", ["server=","port="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            usage()
            sys.exit()
        elif opt in ("-s", "--server"):
            server = arg
        elif opt in ("-p", "--port"):
            port = int(arg)

    XplodifyApp(host=server, port=port).main()


if  __name__ =='__main__':
    main(sys.argv[1:])
