# SIP over Opportunistic Networks #

## Hypotesis ##
The general idea of this project is to have an opportunistic network where nodes in the network send messages to other nodes in the network. The messages transmitted from a source node can have one or more destinations nodes within the network i.e. multicast between nodes. The nodes in the network that relays messages between each other should use the SIP protocol i.e. they are SIP clients. This is illustrated in the Figure below.

![http://wiki.sip-over-oppnet.googlecode.com/git/images/Overview.png](http://wiki.sip-over-oppnet.googlecode.com/git/images/Overview.png)

## What is an Opportunistic Network ##
An opportunistic network contains mobile nodes that are able to communicate wirelessly with each other even though a route connecting them does not exist. All routes from the source node to the destination node are evaluated on a node-to-node basis. This means that nodes are not supposed to possess or acquire topology information related to he network. In short messages are routed through the network and any node can opportunistically be the next hop. This requires that it is likely that the node brings the message closer to the destination.