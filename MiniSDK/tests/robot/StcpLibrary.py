"""Real AF_STCP integration tests. Never substitutes TCP for STCP."""
import ctypes
import importlib.util
import os
from pathlib import Path
import shutil
import secrets
import socket
import struct
import subprocess
import sys
import threading
import time

ROOT = Path(__file__).resolve().parents[2]
_spec = importlib.util.spec_from_file_location('free_sdk_echo_server', ROOT/'python/echo_server.py')
_server_api = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_server_api)
LIBC = ctypes.CDLL(None, use_errno=True)
for name in ('bind', 'connect'):
    fn = getattr(LIBC, name)
    fn.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
    fn.restype = ctypes.c_int


def address(port):
    return ctypes.create_string_buffer(struct.pack('=H', 2) + struct.pack('!H', port)
                                       + socket.inet_aton('127.0.0.1') + bytes(8))


def endpoint(sock, operation, port):
    if getattr(LIBC, operation)(sock.fileno(), address(port), 16) < 0:
        code = ctypes.get_errno()
        raise OSError(code, os.strerror(code))


class StcpLibrary:
    ROBOT_LIBRARY_SCOPE = 'SUITE'

    def __init__(self):
        self.server = None
        self.log = None

    def _run(self, command, timeout=30, cwd=ROOT):
        return subprocess.run(command, cwd=cwd, capture_output=True, text=True,
                              timeout=timeout)

    def prepare_sdk(self, language, port='19700', output_dir='results'):
        self.port = int(port)
        self.port_base = self.port
        assert 1 <= self.port <= 65535, 'Invalid test port'
        self.output = Path(output_dir).resolve()
        self.output.mkdir(parents=True, exist_ok=True)
        # Fail clearly before compiling if the required module is unavailable.
        try:
            with socket.socket(45, socket.SOCK_STREAM, 253):
                pass
        except OSError as exc:
            raise AssertionError(f'AF_STCP/protocol 253 unavailable; load the compatible STCP module: {exc}') from exc
        if language == 'python':
            self.server_cmd = [sys.executable, str(ROOT/'python/echo_server.py')]
            self.client_cmd = [sys.executable, str(ROOT/'python/echo_client.py')]
        elif language == 'rust':
            self._build(['cargo', 'build', '--release', '--offline'], ROOT/'rust', language)
            self.server_cmd = [str(ROOT/'rust/target/release/stcp-free-echo')]
            self.client_cmd = [str(ROOT/'rust/target/release/echo-client')]
        elif language == 'golang':
            for role in ('server', 'client'):
                self._build(['go', 'build', '-o', str(self.output/f'go-echo-{role}'),
                             f'./cmd/echo-{role}'], ROOT/'golang', f'go-{role}')
            self.server_cmd = [str(self.output/'go-echo-server')]
            self.client_cmd = [str(self.output/'go-echo-client')]
        elif language == 'dart':
            for role in ('server', 'client'):
                self._build(['dart', 'compile', 'exe', str(ROOT/f'dart/echo_{role}.dart'),
                             '-o', str(self.output/f'dart-echo-{role}')], ROOT, f'dart-{role}')
            self.server_cmd = [str(self.output/'dart-echo-server')]
            self.client_cmd = [str(self.output/'dart-echo-client')]
        else:
            raise AssertionError(f'Unknown language: {language}')

    def _build(self, command, cwd, label):
        assert shutil.which(command[0]), f'Missing toolchain: {command[0]}'
        result = self._run(command, timeout=300, cwd=cwd)
        (self.output/f'{label}-build.log').write_text(result.stdout + result.stderr)
        assert result.returncode == 0, f'{label} build failed: {result.stdout}{result.stderr}'

    def start_echo_server(self):
        for attempt in range(20):
            self.port = self.port_base + secrets.randbelow(min(20000, 65536-self.port_base))
            self.log = (self.output/'server.log').open('w')
            self.server = subprocess.Popen(self.server_cmd + [str(self.port)], cwd=ROOT,
                                           stdout=self.log, stderr=subprocess.STDOUT)
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                diagnostic = (self.output/'server.log').read_text()
                if self.server.poll() is not None:
                    code = self.server.returncode
                    self.stop_echo_server()
                    if any(marker in diagnostic for marker in
                           ('Address already in use', 'address already in use', 'errno = 98')):
                        break
                    raise AssertionError(f'Server exited (rc={code}):\n{diagnostic}')
                if 'STCP-TCP echo listening on' in diagnostic:
                    return
                time.sleep(0.05)
            else:
                raise AssertionError('Server did not become ready within 10 seconds')
        raise AssertionError('No free STCP server port after 20 attempts')

    def stop_echo_server(self):
        if self.server is not None:
            if self.server.poll() is None:
                self.server.terminate()
                try:
                    self.server.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    self.server.kill()
                    self.server.wait(timeout=3)
            self.server = None
        if self.log:
            self.log.close()
            self.log = None

    def _connect(self):
        sock = socket.socket(45, socket.SOCK_STREAM, 253)
        try:
            endpoint(sock, 'connect', self.port)
            sock.settimeout(5)
            return sock
        except BaseException:
            sock.close()
            raise

    def client_echo_should_pass(self, message='Hello STCP!'):
        result = self._run(self.client_cmd + ['127.0.0.1', str(self.port), message])
        assert result.returncode == 0, (result.stdout + result.stderr
            + f'\nServer status: {self.server.poll() if self.server else None}\n'
            + (self.output/'server.log').read_text())
        expected = f'PASS: {len(message.encode("utf-8"))} bytes echoed unchanged'
        assert expected in result.stdout, result.stdout

    def long_client_message_should_pass(self):
        self.client_echo_should_pass('STCP-ää-0123456789-' * 3000)

    def binary_echo_should_pass(self):
        payload = bytes(range(256)) * 1024
        with self._connect() as sock:
            for offset in range(0, len(payload), 4096):
                part = payload[offset:offset+4096]
                sock.sendall(part)
                reply = bytearray()
                while len(reply) < len(part):
                    data = sock.recv(len(part)-len(reply))
                    assert data, 'Premature EOF'
                    reply.extend(data)
                assert reply == part, 'Binary echo mismatch'

    def repeated_connections_should_pass(self):
        for number in range(10):
            self.client_echo_should_pass(f'connection-{number}')

    def server_should_survive_disconnect(self):
        with self._connect():
            pass
        self.client_echo_should_pass('after-disconnect')
        assert self.server.poll() is None, 'Server died after disconnect'

    def invalid_arguments_should_fail(self):
        for command in (self.server_cmd + ['0'], self.client_cmd + ['127.0.0.1', '0'],
                        self.client_cmd + ['invalid-ip'],
                        self.client_cmd + ['127.0.0.1', str(self.port), '']):
            result = self._run(command)
            assert result.returncode != 0, f'Invalid input accepted: {command}'

    def client_should_detect_bad_echo(self, mode):
        assert mode in ('wrong', 'close', 'fragment')
        # Listener getname is not implemented by this module: bind a known
        # explicit port rather than relying on getsockname() after bind(0).
        with socket.socket(45, socket.SOCK_STREAM, 253) as listener:
            for attempt in range(30):
                port = 20000 + secrets.randbelow(40000)
                try:
                    endpoint(listener, 'bind', port)
                    break
                except OSError as exc:
                    if exc.errno != 98:
                        raise
            else:
                raise AssertionError('No free STCP fixture port after 30 attempts')
            listener.listen(1)
            import select
            errors = []
            def serve():
                try:
                    if not select.select([listener], [], [], 10)[0]:
                        raise AssertionError('STCP fixture accept timed out')
                    with _server_api.accept_client(listener) as peer:
                        peer.settimeout(5)
                        data = bytearray()
                        while len(data) < 10:
                            part = peer.recv(10-len(data))
                            if not part:
                                raise AssertionError('Fixture received incomplete request')
                            data.extend(part)
                        if mode == 'close':
                            return
                        if mode == 'wrong':
                            peer.sendall(b'X' + data[1:])
                        else:
                            for byte in data:
                                peer.sendall(bytes([byte]))
                                time.sleep(0.01)
                except BaseException as exc:
                    errors.append(exc)
            thread = threading.Thread(target=serve, daemon=True)
            thread.start()
            try:
                result = self._run(self.client_cmd + ['127.0.0.1', str(port), '0123456789'], timeout=15)
            finally:
                thread.join(timeout=11)
            assert not thread.is_alive(), 'STCP fixture did not stop'
            assert not errors, f'STCP fixture failure: {errors}'
            if mode == 'fragment':
                assert result.returncode == 0 and 'PASS: 10 bytes' in result.stdout, result.stdout + result.stderr
            else:
                assert result.returncode != 0, 'Client accepted broken echo'
                diagnostic = result.stdout + result.stderr
                if mode == 'wrong':
                    assert 'mismatch' in diagnostic.lower(), diagnostic
                else:
                    assert any(word in diagnostic.lower() for word in ('closed', 'fill whole buffer', 'unexpectedeof')), diagnostic
