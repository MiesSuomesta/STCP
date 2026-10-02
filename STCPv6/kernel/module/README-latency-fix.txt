Based on module-02102026_194315.zip supplied on 2026-10-02.

Extract into /srv/stcp-project/STCP/version-to-use/kernel/module
Only src/stcp_carrier.c and src/stcp_ops.c replace module files.
Rebuild and reload host module with your usual build/install workflow.
The unified source change also applies to the Raspberry build.

Fix: use interruptible waits on TCP sk_sleep and STCP accept_wq, matching their interruptible wakeups.
Handle interrupted waits before interpreting the result as success.
Retain the existing 750 ms drain, 1250 ms FIN and 100 ms accept limits.
No timeout reduction, wire format change, crypto change, Rust change or UI change.
Remote/external accepts without a local Rust child can still use the 100 ms fallback.

Verified: applies to the supplied current source; only the two source files changed.
A kernel build/run was not possible here because matching kernel headers are absent.
After rebuild/reload, rerun strace -tt -T on the echo client and compare connect/close durations.
Then run Linux/local and Linux/Raspberry TCP/UDP regression tests plus repeated connection churn.
