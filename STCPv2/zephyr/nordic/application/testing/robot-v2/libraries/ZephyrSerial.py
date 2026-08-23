import json
import re
import time

try:
    import serial
except ImportError as exc:
    raise RuntimeError("pyserial is required: pip install pyserial") from exc

class ZephyrSerial:
    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self.ser = None

    def open_zephyr_serial(self, device, baud=115200):
        self.ser = serial.Serial(device, int(baud), timeout=0.1)
        self.ser.reset_input_buffer()
        self.ser.write(b"\r\n")
        return device

    def close_zephyr_serial(self):
        if self.ser is not None:
            self.ser.close()
            self.ser = None

    def _read_until(self, predicate, timeout):
        end = time.monotonic() + float(timeout)
        data = ""
        while time.monotonic() < end:
            chunk = self.ser.read(4096)
            if chunk:
                data += chunk.decode("utf-8", errors="replace")
                if predicate(data):
                    return data
            else:
                time.sleep(0.02)
        raise AssertionError(f"timeout after {timeout}s; received:\n{data[-8000:]}")

    def run_shell_command(self, command, expect="stcp>", timeout=10):
        if self.ser is None:
            raise AssertionError("serial is not open")
        self.ser.reset_input_buffer()
        self.ser.write((command + "\r\n").encode())
        return self._read_until(lambda s: expect in s, timeout)

    def run_benchmark(self, command, timeout=90):
        if self.ser is None:
            raise AssertionError("serial is not open")
        self.ser.reset_input_buffer()
        self.ser.write((command + "\r\n").encode())
        text = self._read_until(lambda s: "STCP_BENCH_JSON_END" in s, timeout)
        parts = re.findall(r"STCP_BENCH_JSON_PART\s+(.*)", text)
        if not parts:
            raise AssertionError("benchmark JSON not found\n" + text[-8000:])
        payload = "".join(p.strip() for p in parts)
        result = json.loads(payload)
        if result.get("status") != 0 or result.get("errors") != 0:
            raise AssertionError(f"benchmark failed: {result}\n{text[-8000:]}")
        return result
