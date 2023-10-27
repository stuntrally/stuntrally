### Introduction

This page contains information about the network multiplayer feature.

### Quick guide

1.  Alternatively you can host your own game by pressing the
    `Create game` button (the game will show up in the list for others
    to join).
    -   You can also connect directly to an IP:port by pressing
        `Direct connect`
2.  Press `Ready` button in the game lobby tab and wait for others to
    become ready too
3.  The game shall start when the host presses the button

<!-- -->

1.  It is possible for host to continue game on other track (or restart
    on current)
    -   When host presses new game (same button as start) current game
        will end, peers will have shown menu
    -   Host can now change track (and game settings) and after all
        peers have pressed ready, game can start again

  
### Direct connect

Networked multiplayer only works with Direct connect (all players need
to specify IP). Doesn't need to be only local network.

To play multiplayer you should enlist your friends to play with you. All
players need to have ports forwarded, and use Direct connect.

## Master server (unavailable)

If you see on Multiplayer tab you can see an error message "Couldn't
reach the master server". This is because it's not working since years
and won't be. Unless someone can host it on an IP that is not changing
or better has a named domain. In that case please post on our forum.

The list of available games will never be filled. And when it was there
were rarely games in it.

But even years ago (when it worked) it was rarely not empty. There was
also a webpage with status of available games back then. And you could
come to #stuntrally on Freenode to challenge one of the hang-arounds.

  

### Troubleshooting

**Q:** I get a "Network error" message box when I try to join or create
a game.

**A:** Maybe some other program is currently using the local port. Try a
different one by changing it in the `Settings` tab. Also make sure your
firewall isn't blocking Stunt Rally - it needs permissions to both
access the internet as well as listen for connections (i.e. permission
to be a server).

  
**Q:** Other players can't connect to me

**A:** Most likely your router is blocking access. Configure it to
forward the traffic coming to the Stunt Rally local port (check it from
Multiplayer's `Settings` tab) to your computer. You might find
<http://portforward.com/> helpful.

  
**Q:** I cannot connect to some players

**A:** This is the previous problem reversed - their firewall is
misconfigured. If you have your router properly configured, the others
should be able to connect to you and as such you don't need to connect
to them anymore. Obviously this doesn't apply if you are the host.

  
**Q:** I get errors about protocol versions

**A:** This means that your version of Stunt Rally is incompatible with
the master-server and/or other players (depending on with whom you are
trying to communicate). Your best bet is to make sure you are running
the latest release of SR. Otherwise, tell your fellow players or
master-server administrator (that's us most likely) to upgrade.

  
**Q:** How can I change my nickname?

**A:** Go to the `Settings` tab under the Multiplayer main tab.

  

### Technical details

SR networking is based on the peer-to-peer architecture, meaning all
players are connected to each other and there is no separate game
(simulating) server.

The underlying protocol used is UDP, through ENet (networking
library).  
SR builds it's own layer on top of that, so using generic UDP sockets
for communicating with SR or master server is not possible.

### Master (game) server

Stunt Rally **used** a lightweight master server for tracking available
games.

Master server is a simple command-line server (can be daemonized on Linux)  
that accepts game announcements and passes a list of them to anyone who asks.  
Clients must update the games periodically or they will timeout in the master server.

Anyone can build and **host** a master server. Info also [here](https://github.com/stuntrally/stuntrally/blob/master/source/network/DesignDoc.txt).  
and its code [here](https://github.com/stuntrally/stuntrally/blob/master/source/network/master-server/main.cpp) 
it builds with CMake in SR repo.

Players need to set the correct address and port in the Multiplayer -
`Settings` tab in order to connect to it and use.

If master server works:

1.  Host creates a game on Single Race - Multiplayer with `Create game`
    button (just like for Direct connect).
2.  Then others to join game: Go to the Single Race - Multiplayer tab
    and press the `Refresh list` button.
    -   This will connect to master server and should list game(s)
3.  Select a game you wish to join and press the `Join game` button
