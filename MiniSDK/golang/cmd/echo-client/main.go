// Linux STCP-TCP echo client; standard library only.
package main
import (
    "bytes"
    "fmt"
    "net"
    "os"
    "strconv"
    "syscall"
)
func run() error {
    if len(os.Args) < 2 || len(os.Args) > 4 { return fmt.Errorf("usage: echo-client IPv4 [port] [message]") }
    ip := net.ParseIP(os.Args[1]).To4()
    if ip == nil { return fmt.Errorf("IPv4 literal required") }
    port := 7000
    if len(os.Args) > 2 { value, err := strconv.Atoi(os.Args[2]); if err != nil { return err }; port = value }
    if port < 1 || port > 65535 { return fmt.Errorf("port must be 1..65535") }
    payload := []byte("Hello STCP!")
    if len(os.Args) > 3 { payload = []byte(os.Args[3]) }
    if len(payload) == 0 { return fmt.Errorf("message must not be empty") }
    fd, err := syscall.Socket(45, syscall.SOCK_STREAM, 253)
    if err != nil { return err }; defer syscall.Close(fd)
    address := &syscall.SockaddrInet4{Port: port}; copy(address.Addr[:], ip)
    if err := syscall.Connect(fd, address); err != nil { return err }
    for start := 0; start < len(payload); start += 4096 {
        end := start + 4096; if end > len(payload) { end = len(payload) }
        part := payload[start:end]
        for offset := 0; offset < len(part); {
            n, err := syscall.SendmsgN(fd, part[offset:], nil, nil, syscall.MSG_NOSIGNAL)
            if err == syscall.EINTR { continue }; if err != nil { return err }
            if n == 0 { return fmt.Errorf("zero-byte send") }; offset += n
        }
        reply := make([]byte, len(part))
        for offset := 0; offset < len(reply); {
            n, err := syscall.Read(fd, reply[offset:])
            if err == syscall.EINTR { continue }; if err != nil { return err }
            if n == 0 { return fmt.Errorf("connection closed before complete echo") }; offset += n
        }
        if !bytes.Equal(part, reply) { return fmt.Errorf("echo mismatch") }
    }
    fmt.Printf("PASS: %d bytes echoed unchanged\n", len(payload)); return nil
}
func main() { if err := run(); err != nil { fmt.Fprintln(os.Stderr, err); os.Exit(1) } }
