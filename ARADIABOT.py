#!/usr/bin/python

import socket
 
HOST="irc.rizon.net"
PORT=6667
NICK="ARADIABOT"
REALNAME="Aradia Megido"
CHANNEL="#iridia"
 
s = socket.socket()
s.connect((HOST, PORT))
s.send("NICK %s\r\n" % NICK)
s.send("USER %s %s bla :%s\r\n" % (NICK, HOST, REALNAME))
s.send("JOIN %s\r\n" % CHANNEL)
 
readbuffer = ""

while (1):
    readbuffer=readbuffer+s.recv(1024)
    temp=readbuffer.split("\n")
    readbuffer=temp.pop( )
    for line in temp:
        print(line)
        if (len(line.split()) > 3):
            line = line.split( )[3]

            if(line==":PING"):
                print("Ping request!")
                pong = "PONG %s\r\n" % line[4:len(line)-1]
                s.send(pong)
                break

            if(line==":!r"):
                print("ribbit")
                reply ="PRIVMSG " + CHANNEL + " :ribbit\r\n"
                s.send(reply)
                break
