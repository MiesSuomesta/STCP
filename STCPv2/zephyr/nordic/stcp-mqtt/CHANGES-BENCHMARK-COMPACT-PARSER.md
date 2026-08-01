# Benchmark compact result parser fix

- Firmware now terminates each `STCP_RESULT` record with a real newline instead of the literal characters `\\n`.
- Host runner removes ANSI terminal control sequences before parsing.
- Compact result parsing uses a strict regular expression, so a following Zephyr log line cannot become part of the final numeric field.
- Duplicate `STCP_RESULT` records remain harmless; the first complete valid record ends the case.
