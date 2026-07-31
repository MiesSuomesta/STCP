# Benchmark result status and multipart JSON fix

- Multipart records now contain a raw JSON object (`{...}`), so the host runner can pass the reassembled payload directly to `json.loads()`.
- Upload, download and full-duplex early failures now update the matching `last_summary` entry before returning.
- The machine-readable `status` and `errors` fields now match the shell-visible benchmark return code, including allocation, DNS/connect and initial request-send failures.
- W5500 driver sources and workarounds are not modified by this package.
