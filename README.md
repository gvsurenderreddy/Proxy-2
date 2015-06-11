# Proxy
## Introduction</br>

Proxy is a client-server application that creates a SSL tunnel between two endpoints. The server side is an UNIX daemon (also can be executed as a console application) that listens for connections from the client and forwards all the traffic when the connection is established. The client, once it has established a connection to the server, will create a TUN interface with an IPv6 address and an IPv4 address, and will attempt to adjust the routing table to route all traffic through the newly created tunnel, if configured to do so (see the command line options). That is, once the tunnel is created, all client's traffic will be routed through the SSL tunnel to the server, which will behave as an IPv6 and IPv4 proxy.</br>
</br>
The application of proxy is to facilitate access to Internet in different ways through a secure SSL tunnel. For example access  IPv6 and IPv4 Internet from different locations, those where the server side is running. Another use case is to access Internet using IPv6 (if your ISP has not stepped into 21st century and is stuck with IPv4 yet) creating a tunnel from an IPv4 machine to another IPv6 capable machine. That is, running the client side in an IPv4 box towards a server, somewhere else, with IPv6 connectivity -and double stack- gives automatic IPv6 connectivity to the client. Now you can enjoy IPv6. Other use cases are left to the imagination of the reader.</br>
</br>
## <a href="https://www.politeping.com/9/" target="_blank"> Continue reading</a>
