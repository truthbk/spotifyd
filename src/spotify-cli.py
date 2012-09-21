#!/usr/bin/env python

import sys
sys.path.append('../gen-py')

from spotify import Spotify
from spotify.ttypes import *
from spotify.constants import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

import curses

SPOTIFYD_PORT = 9090

def get_param(prompt_string):
    screen.clear()
    screen.border(0)
    screen.addstr(2, 2, prompt_string)
    screen.refresh()
    input = screen.getstr(10, 10, 60)
    return input

def spot_login(client):
    pass

def spot_selplaylist(client):
    pass

def spot_seltrack(client):
    pass

def spot_playback(client):
    pass

def spot_getcurrent(client):
    pass

def spot_logout(client):
    pass


def main():
    try:
        myscreen = curses.initscr()
        myscreen.border(0)


        # Make socket
        transport = TSocket.TSocket('localhost', SPOTIFYD_PORT)

        # Buffering is critical. Raw sockets are very slow
        transport = TTransport.TBufferedTransport(transport)

        # Wrap in a protocol
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        # Create a client to use the protocol encoder
        client = Spotify.Client(protocol)

        # Connect!
        transport.open()

        opt = 0

        while opt is not '6':

            screen.addstr(2, 2, "What d'you wanna do??")
            screen.addstr(4, 4, "1. Login")
            screen.addstr(5, 4, "2. Select Playlist")
            screen.addstr(6, 4, "3. Select Track")
            screen.addstr(7, 4, "4. Control Playback")
            screen.addstr(8, 4, "5. Get current track")
            screen.addstr(9, 4, "6. Logout")

            myscreen.getch()

            result = {
                    '1': spot_login(),
                    '2': spot_selplaylist(),
                    '3': spot_seltrack(),
                    '4': spot_playback(),
                    '5': spot_getcurrent(),
                    '6': spot_logout()
            }[opt]()

        """
        SAMPLE THRIFT CALL CODE

        client.ping()
        print "ping()"

        msg = client.sayHello()
        print msg
        msg = client.sayMsg(HELLO_IN_KOREAN)
        print msg

        """
        transport.close()

    except Thrift.TException, tx:
        print "%s" % (tx.message)
