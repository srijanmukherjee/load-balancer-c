# Load Balancer in C

A very basic implementation of a round robin load balancer using C. It redirects request to
predefined list of servers using the round robin policy.

This is for demonstration and learning purpose only, and thus used fork() to quickly handle multiple clients instead of using proper techniques like polling.

From the demonstration we learned that the `proxy <-> remote` and `client <-> proxy` sockets do not close by themselves automatically by default. There are a lot of possiblities depending on the different header values. For example, the `Keep-Alive` header is set to `timeout=5`, which kept the server socket open for 5secs. If a request went in between those 5sec, the timeout is reset and the same pair of `remote <-> proxy` and `client <-> proxy` sockets are used because the `remote <-> proxy` socket doesn't close for 5 seconds, which in turn kept the `client <-> proxy` socket open.

I haven't tested this with SSL connection yet, which would be interesting to test and implement. This could potentially work if each server indivually handled SSL. But I am thinking of handling the SSL until the proxy server and then forward the decoded http request to the *other* servers. Kindof like nginx.

I would also like to assign a custom server name in the `Server` header field. I will need handle http headers in C which seems like a big challenge in itself. I may revisit this later in the future, maybe, in a higher level language.