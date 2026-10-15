"""Local echo-loop tests, independent of kernel module and Robot."""
import importlib.util
from pathlib import Path
import socket
import threading
import unittest
from unittest.mock import patch
import ctypes
import errno

ROOT = Path(__file__).resolve().parents[2]
def load(name):
    spec = importlib.util.spec_from_file_location(name, ROOT/f'python/echo_{name}.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module
client, server = load('client'), load('server')

class EchoTests(unittest.TestCase):
    def exchange(self, payload, mode='normal'):
        left, right = socket.socketpair()
        left.settimeout(3)
        right.settimeout(3)
        errors = []
        def serve():
            try:
                with right:
                    if mode == 'normal':
                        server.echo(right)
                        return
                    while data := right.recv(4096):
                        if mode == 'close':
                            return
                        if mode == 'wrong':
                            right.sendall(b'X' + data[1:])
                            return
                        for offset in range(0, len(data), 7):
                            right.sendall(data[offset:offset+7])
            except BaseException as error:
                errors.append(error)
        thread = threading.Thread(target=serve, daemon=True)
        thread.start()
        try:
            with left:
                client.exchange(left, payload)
        finally:
            thread.join(4)
            self.assertFalse(thread.is_alive(), 'echo fixture hung')
            self.assertEqual(errors, [])

    def test_raw_accept_descriptor(self):
        with socket.socket() as listener:
            listener.bind(('127.0.0.1', 0))
            listener.listen(1)
            with socket.create_connection(listener.getsockname()) as peer:
                with server.accept_client(listener) as accepted:
                    peer.sendall(b'raw-accept')
                    self.assertEqual(accepted.recv(10), b'raw-accept')
                    accepted.sendall(b'ok')
                    self.assertEqual(peer.recv(2), b'ok')

    def test_accept_retries_connection_errors(self):
        left, right = socket.socketpair()
        fd = left.detach()
        codes = iter([errno.ECONNRESET, errno.ECONNABORTED, errno.EINTR,
                      errno.ENOTCONN, errno.EPIPE, errno.ETIMEDOUT, errno.ECONNREFUSED])
        attempts = []
        def accept(*args):
            attempts.append(1)
            if len(attempts) <= 7:
                ctypes.set_errno(next(codes))
                return -1
            return fd
        try:
            with patch.object(server.libc, 'accept', side_effect=accept):
                with server.accept_client(right):
                    self.assertEqual(len(attempts), 8)
        finally:
            right.close()

    def test_ascii(self): self.exchange(b'Hello STCP!')
    def test_unicode(self): self.exchange('Terve ääkköset 日本 😀'.encode())
    def test_binary_256k(self): self.exchange(bytes(range(256))*1024)
    def test_fragmented(self): self.exchange(bytes(range(256))*32, 'fragment')
    def test_mismatch(self):
        with self.assertRaisesRegex(ValueError, 'mismatch'):
            self.exchange(b'hello', 'wrong')
    def test_premature_eof(self):
        with self.assertRaisesRegex(OSError, 'closed'):
            self.exchange(b'hello', 'close')

if __name__ == '__main__': unittest.main()
