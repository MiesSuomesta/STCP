# STCP server accept fix v4

## Root cause

CPython `socket.accept()` / `socket._accept()` tries to decode the peer address after the kernel accept succeeds. AF_STCP (45) is not in CPython's sockaddr table, so it raises `OSError: getsockaddrlen: bad family`.

## Fix

STCP uses `accept4(fd, NULL, NULL, 0)` (or `accept`) through ctypes. No sockaddr is requested, so Python never tries to decode AF_STCP. The returned fd is wrapped explicitly as an AF_STCP socket.

The Raspberry sync command also touches copied files after extraction to avoid misleading future-timestamp warnings when host clocks differ.
