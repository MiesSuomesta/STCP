# W5500 patcher tabs fix

- Fixed the fast-polling patch generator to emit actual indentation instead of literal `\\t` text in C source.
- The patcher now detects an earlier broken literal-`\\t` driver patch and automatically restores `eth_w5500.c.stcp-original` before applying the corrected patch.
- Existing clean fast-polling patches remain idempotent.
