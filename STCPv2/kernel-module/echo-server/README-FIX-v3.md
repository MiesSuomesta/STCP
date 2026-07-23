# Echo benchmark v3 fix

The Zephyr socket is now explicitly put into `O_NONBLOCK` mode. The previous v2
implementation combined blocking `SO_RCVTIMEO` with a poll deadline. A blocking
`recv()` could consume the entire timeout before `poll()` was reached, producing
`-ETIMEDOUT` immediately after `EAGAIN`.

v3 uses one timeout mechanism only: nonblocking send/recv plus `zsock_poll()` and
a monotonic operation deadline. The LTE-oriented default operation timeout is 60 s.
