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

        # init curses screen
        self._screen = curses.initscr()
        self._screen.border(0)



    def get_param(self, prompt_string, passwd=False):
        self._screen.clear()
        self._screen.border(0)
        self._screen.addstr(2, 2, prompt_string)
        self._screen.refresh()
        if passwd:
            curses.noecho()
        input = self._screen.getstr(2, 15, 60)
        curses.echo()
        return input

    def spot_login(self):
        username = self.get_param("username: ")
        password = self.get_param("password: ", True)

        try:
            credentials = SpotifyCredential( username, password )
            self._credentials = self._client.loginSession(credentials)
        except Exception, e:
            self._screen.clear()
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
           self._screen.addstr(30, 2, e.__str__())
           self._success = False
       finally:
           return ret

    def spot_getplaylists(self):
        try:
            self._screen.clear()
            self._screen.border(0)
            """ pls will be a set with the playlists """
            pls = self._client.getPlaylists(self._credentials)
            row = 3
            for p in pls:
                self._screen.addstr(row, 3, p) 
                row += 1

            self._screen.getch()
            self._screen.refresh()

        except Exception, e:
            self._screen.addstr(30, 2, e.__str__())
            self._success = False
            return None


    def spot_selplaylist(self):
        return None

    def spot_seltrack(self):
        return None

    def spot_playback(self):
        return None

    def spot_getcurrent(self):
        return None

    def spot_logout(self):
        return None

    def menu(self):
        try:

            opt = 0
            funcs = {
                    ord('1'): self.spot_login, 
                    ord('2'): self.spot_getplaylists, 
                    ord('3'): self.spot_selplaylist,
                    ord('4'): self.spot_seltrack,
                    ord('5'): self.spot_playback,
                    ord('6'): self.spot_getcurrent,
                    ord('7'): self.spot_logout,
            }

            while opt is not ord('8'):
                if(self._success):
                    self._screen.clear()
                if self._credentials:
                    self._screen.addstr(2, 2, "Logged in as %s" % self._credentials._username )
                self._screen.addstr(4, 2, "What d'you wanna do??")
                self._screen.addstr(6, 4, "1. Login")
                self._screen.addstr(7, 4, "2. Get Playlists")
                self._screen.addstr(8, 4, "3. Select Playlist")
                self._screen.addstr(9, 4, "4. Select Track")
                self._screen.addstr(10, 4, "5. Control Playback")
                self._screen.addstr(11, 4, "6. Get current track")
                self._screen.addstr(12, 4, "7. Logout")
                self._screen.addstr(13, 4, "8. Exit")
                self._screen.addstr(15, 4, "Option: ")

                opt = self._screen.getch()

                if opt>ord('0') and opt<ord('8'):
                    result = funcs[opt]()

            self._transport.close()
            curses.endwin()

        except Thrift.TException, tx:
            curses.endwin()
            print "%s" % (tx.message)


def main():

    spoticlient = spclient()
    signal.signal(signal.SIGINT, signal_handler)

    spoticlient.menu()

    """
    SAMPLE THRIFT CALL CODE

    client.ping()
    print "ping()"

    msg = client.sayHello()
    print msg
    msg = client.sayMsg(HELLO_IN_KOREAN)
    print msg

    """

if  __name__ =='__main__':main()
