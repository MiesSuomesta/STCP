STCP SDK Robot SIGWINCH fix

Problem:
strace-wrapped blocking connect() could receive SIGWINCH. The syscall was
restarted after it had already progressed to connected state, producing EISCONN.

Fix:
- local traced server/client processes ignore SIGWINCH via preexec_fn
- remote traced server/client commands execute with `trap '' WINCH`
- no kernel/shared-core changes
- test timeout remains 45 seconds

Extract at SDK/v2 root.
