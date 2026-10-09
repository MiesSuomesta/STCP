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

    def _drain_input(self, quiet_time=0.15, max_time=1.0):
        """
        Drain currently pending serial output without throwing away data that
        belongs to a command we have already sent.

        Wait until the line has been quiet for quiet_time, but never longer
        than max_time. This is used only before starting a new command.
        """
        end = time.monotonic() + float(max_time)
        quiet_deadline = time.monotonic() + float(quiet_time)
        data = ""

        while time.monotonic() < end:
            chunk = self.ser.read(4096)
            if chunk:
                data += chunk.decode("utf-8", errors="replace")
                quiet_deadline = time.monotonic() + float(quiet_time)
                continue

            if time.monotonic() >= quiet_deadline:
                break

            time.sleep(0.01)

        return data

    def _sync_prompt(self, prompt="stcp>", timeout=3.0):
        """
        Synchronize to a fresh Zephyr shell prompt.

        We first drain old asynchronous output, then send an empty line and
        wait for the *new* prompt produced by that line. This prevents one
        Robot test from consuming the previous test's trailing console data.
        """
        self._drain_input()

        self.ser.write(b"\r\n")
        self.ser.flush()

        text = self._read_until(lambda s: prompt in s, timeout)

        # Consume anything that was emitted immediately after the prompt so
        # the next command starts from a quiet serial line.
        tail = self._drain_input(quiet_time=0.05, max_time=0.25)
        return text + tail

    def run_shell_command(self, command, expect="stcp>", timeout=10):
        if self.ser is None:
            raise AssertionError("serial is not open")

        self._sync_prompt("stcp>", timeout=min(float(timeout), 3.0))

        self.ser.write((command + "\r\n").encode())
        self.ser.flush()

        text = self._read_until(lambda s: expect in s, timeout)

        # Drain short asynchronous tail after the command's terminating
        # prompt so it cannot bleed into the next Robot test.
        text += self._drain_input(quiet_time=0.05, max_time=0.25)
        return text

    def run_benchmark(self, command, timeout=90):
        if self.ser is None:
            raise AssertionError("serial is not open")

        # Start every benchmark from a known shell boundary.
        self._sync_prompt("stcp>", timeout=3.0)

        self.ser.write((command + "\r\n").encode())
        self.ser.flush()

        # The JSON end marker is the benchmark completion marker, but the
        # shell prompt is emitted only after control returns to the shell.
        # Consume both before starting the next Robot test.
        text = self._read_until(lambda s: "STCP_BENCH_JSON_END" in s, timeout)

        try:
            text += self._read_until(lambda s: "stcp>" in s, 3.0)
        except AssertionError:
            # Some benchmark implementations print the prompt in the same
            # serial chunk as the JSON end marker. Check the complete text
            # before declaring prompt synchronization lost.
            if "stcp>" not in text:
                raise AssertionError(
                    "benchmark completed but shell prompt did not return\n"
                    + text[-8000:]
                )

        text += self._drain_input(quiet_time=0.05, max_time=0.25)

        parts = re.findall(r"STCP_BENCH_JSON_PART\s+(.*)", text)
        if not parts:
            raise AssertionError("benchmark JSON not found\n" + text[-8000:])

        payload = "".join(p.strip() for p in parts)
        result = json.loads(payload)

        if result.get("status") != 0 or result.get("errors") != 0:
            raise AssertionError(f"benchmark failed: {result}\n{text[-8000:]}")

        return result
