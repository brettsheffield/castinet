[![Build Status](https://travis-ci.org/brettsheffield/castinet.svg?branch=master)](https://travis-ci.org/brettsheffield/castinet)

castinet - small ipv6-only ASM/SSM mode multicast source and listener test programs

Just a really simple pair of programs for testing multicast reachability.

The source program (castinet) sends the specified message to the multicast address and port provided once per second until stopped with a CTRL-c / SIGINT.

The listener program listens for inbound multicast messages and prints them as they arrive.

Usage:

 castinet [--addr multicast address] [--port multicast port] [--grp groupname] [--ttl ttl] [--loop 0|1] message

 eg. castinet --addr ff3e::1 --port 4242 `hostname`

 listener [--addr multicast address] [--port port] [--grp groupname] [--src source address]

Providing a multicast source address to the listener with --src switches to Single Source Multicast (SSM) mode from the default Any Source Multicast (ASM) mode.

Multiple multicast groups can be joined by supplying more than one --addr argument.

 --grp groupname

the groupname is hashed to find the multicast group to join


Brett Sheffield `<brett@gladserv.com>`

2017-01-25
