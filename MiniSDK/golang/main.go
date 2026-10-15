// Linux STCP-TCP echo server. Standard library only.
package main

import (
    "fmt"
    "os"
    "strconv"
    "syscall"
)

func echo(fd int) error {
    buffer := make([]byte, 65536)
    for {
        n, err := syscall.Read(fd, buffer)
        if err == syscall.EINTR { continue }
        if err != nil { return err }
        if n == 0 { return nil }
        for offset := 0; offset < n; {
            // MSG_NOSIGNAL prevents a disconnected client killing the server.
            sent, err := syscall.SendmsgN(fd, buffer[offset:n], nil, nil, syscall.MSG_NOSIGNAL)
            if err == syscall.EINTR { continue }
            if err != nil { return err }
            if sent == 0 { return fmt.Errorf("zero-byte send") }
            offset += sent
        }
    }
}
func run() error {
    if len(os.Args) > 2 { return fmt.Errorf("usage: stcp-free-echo [port]") }
    port := 7000
    if len(os.Args) == 2 {
        value, err := strconv.Atoi(os.Args[1]); if err != nil { return err }; port = value
    }
    if port < 1 || port > 65535 { return fmt.Errorf("port must be 1..65535") }
    fd, err := syscall.Socket(45, syscall.SOCK_STREAM, 253)
    if err != nil { return err }
    defer syscall.Close(fd)
    if err := syscall.Bind(fd, &syscall.SockaddrInet4{Port: port}); err != nil { return err }
    if err := syscall.Listen(fd, 16); err != nil { return err }
    fmt.Printf("STCP-TCP echo listening on 0.0.0.0:%d\n", port)
    for {
        client, _, err := syscall.Accept(fd)
        if err == syscall.EINTR { continue }
        if err != nil { return err }
        err = echo(client)
        syscall.Close(client)
        if err != nil { fmt.Fprintln(os.Stderr, "client:", err) }
    }
}
func main() { if err := run(); err != nil { fmt.Fprintln(os.Stderr, err); os.Exit(1) } }
