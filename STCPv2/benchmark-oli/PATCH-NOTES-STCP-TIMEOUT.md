# STCP benchmark timeout fix

Changes:

- AF_STCP remains blocking; Python `settimeout()` is not used because it enables
  `O_NONBLOCK`.
- STCP send and receive operations now wait through `select()` with the
  configured `--timeout` deadline.
- A failed connection aborts the client start barrier so sibling workers cannot
  remain blocked forever.
- Every benchmark case has a hard outer process deadline using GNU `timeout`.
- Failed or timed-out cases are reported and the matrix continues by default.

Environment variables:

- `STCP_OPERATION_TIMEOUT=30`
- `CASE_GRACE_SECONDS=20`
- `CONTINUE_CASE_ON_ERROR=1`

Set `CONTINUE_CASE_ON_ERROR=0` to restore fail-fast behavior.
