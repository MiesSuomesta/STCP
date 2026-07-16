# Large test fix

The server now receives and validates all 204800 bytes before sending the OK reply.
Previously the receive loop was missing, so the server printed `verified 0`, closed the
connection while the client was still sending, and the client died with SIGPIPE (make
error 141). The test also ignores SIGPIPE and uses MSG_NOSIGNAL for the server reply.
