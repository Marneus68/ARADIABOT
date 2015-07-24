# ARADIABOT

Simple and small IRC bot written in C++11.  

Whith this bot, the users of your channel that register to it will be able to obtain an rundown of what was said and happened while they weren't connected.   

## Building

On any standard UNIX system with a g++ versuon that supports C++11, you can compile `ARADIABOT` simply by "running" the souce.   
In the freshly clonned directory, execute the `ARADIABOT.cpp`.

    ./ARADIABOT.cpp

If everything is fine, this should produce a binay named `ARADIABOT` in the same directory.  

If you want to use a different compiler, modify the first line of the source file and replace the path to g++ to the compiler you want to use.   

## Running

ARADIABOT expects seveal command line parameters:  

    ARADIABOT <server> <port> <channel> <bot name>

## Using

ARADIABOT can be used by sending it private messages. The commands most users will need are the following:

    REGISTER
    
Register the user sending the message to the history service.

    UNREGISTER

Unregister the user sending the message from the history service.

    HISTORY

Sends the user all public messages that he missed while he was away.

    HELP

Gives you a rundown of all available commands.

## License

ARADIABOT is under the WTFPL. See LICENSE for more details.  
