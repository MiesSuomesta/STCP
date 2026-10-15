"""Exercise Robot fault fixtures over local TCP; integration still uses STCP."""
import importlib.util
from pathlib import Path
import socket
import subprocess
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('robot_helpers', ROOT/'tests/robot/StcpLibrary.py')
helpers = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helpers)
spec = importlib.util.spec_from_file_location('echo_client', ROOT/'python/echo_client.py')
client = importlib.util.module_from_spec(spec)
spec.loader.exec_module(client)

class FixtureTests(unittest.TestCase):
    def test_fault_fixtures_without_listener_getsockname(self):
        original_socket = socket.socket
        library = helpers.StcpLibrary()
        library.client_cmd = ['test-client']
        def run(command, **kwargs):
            try:
                with socket.create_connection(('127.0.0.1', int(command[-2])), timeout=3) as sock:
                    client.exchange(sock, command[-1].encode())
                return subprocess.CompletedProcess(command, 0, 'PASS: 10 bytes', '')
            except (ValueError, OSError) as exc:
                return subprocess.CompletedProcess(command, 1, '', str(exc))
        library._run = run
        def bind(sock, operation, port):
            self.assertEqual(operation, 'bind')
            self.assertNotEqual(port, 0)
            sock.bind(('127.0.0.1', port))
        def new_socket(*args, **kwargs):
            # Only this isolated unit test substitutes TCP for STCP.
            if args and args[0] == 45:
                return original_socket(socket.AF_INET, socket.SOCK_STREAM)
            return original_socket(*args, **kwargs)
        with patch.object(helpers, 'endpoint', side_effect=bind), \
             patch.object(socket, 'socket', side_effect=new_socket), \
             patch.object(helpers.LIBC, 'getsockname', side_effect=AssertionError('must not query listener')):
            for mode in ('wrong', 'close', 'fragment'):
                with self.subTest(mode=mode):
                    library.client_should_detect_bad_echo(mode)
