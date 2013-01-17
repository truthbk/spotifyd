#!/usr/bin/env python

import sys
sys.path.append('./gen-py')

from spotify import Spotify
from spotify.ttypes import *
from spotify.constants import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

import curses

SPOTIFYD_PORT = 9090


class SpClient(object):

    def __init__(self):
        # init curses screen
        self._screen = curses.initscr()
        self._screen.border(0)


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

    def get_param(self, prompt_string):
        self._screen.clear()
        self._screen.border(0)
        self._screen.addstr(2, 2, prompt_string)
        self._screen.refresh()
        input = self._screen.getstr(10, 10, 60)
        return input

    def spot_login(self):
        username = self.get_param("username: ")
        password = self.get_param("password: ")

        credentials = SpotifyCredential( username, password)
        self._client.loginSession(credentials)
        return credentials


    def spot_selplaylist(self, client):
        pass

    def spot_seltrack(self, client):
        pass

    def spot_playback(selfi, client):
        pass

    def spot_getcurrent(self, client):
        pass

    def spot_logout(self, client):
        pass

    def menu(self):
        try:

            opt = 0

            while opt is not '6':
                self._screen.clear()
                self._screen.addstr(2, 2, "What d'you wanna do??")
                self._screen.addstr(4, 4, "1. Login")
                self._screen.addstr(5, 4, "2. Select Playlist")
                self._screen.addstr(6, 4, "3. Select Track")
                self._screen.addstr(7, 4, "4. Control Playback")
                self._screen.addstr(8, 4, "5. Get current track")
                self._screen.addstr(9, 4, "6. Logout")
                self._screen.addstr(12, 4, "Option: ")

                opt = self._screen.getch()

                result = {
                        '1': self.spot_login(),
                        '2': self.spot_selplaylist(),
                        '3': self.spot_seltrack(),
                        '4': self.spot_playback(),
                        '5': self.spot_getcurrent(),
                        '6': self.spot_logout()
                }[opt]()

            transport.close()
            curses.endwin()

        except Thrift.TException, tx:
            print "%s" % (tx.message)


def main():
    spoticlient = SpClient()

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
