# ARADIABOT

Simple and small IRC bot written in C++11.  

With this bot, the users of your channel that register will be able to obtain a rundown of what was said and happened while they weren't connected.   

## Building

On any standard UNIX system with a g++ versuon that supports C++11, you can compile `ARADIABOT` simply by "running" the source file.   
In the freshly cloned directory, execute the `ARADIABOT.cpp`.

    ./ARADIABOT.cpp

If everything is fine, this should produce a binary named `ARADIABOT` in the same directory.  

If you want to use a different compiler, modify the first line of the source file and replace the path to g++ to the compiler you want to use.   

## Running

ARADIABOT expects several command line parameters:  

    ARADIABOT <server> <port> <channel> <bot name>

## Using

### Simple usage

To use this bot, you must register to it by sending it a private message containing the REGISTER command. Once registered, you will be able to get all messages you missed next time you connect to the channel. To get all the history of things that happened in your absence, send the HISTORY command to the bot in a private message.

### List of commands

ARADIABOT can be used by sending it private messages. The commands most users will need are the following:

    REGISTER
    
Register the user sending the message to the history service.

    UNREGISTER

Unregister the user sending the message from the history service.

    HISTORY

Sends the user all public messages that he missed while he was away.

    HELP

Gives you a rundown of all available commands.

## Byproduct

By running ARADIABOT, two files will be created by the executable. Those files, named `history.log` and `users.log`, containing the channel history and the list of users, respectively.

## License

ARADIABOT is under the WTFPL. See LICENSE for more details.  
