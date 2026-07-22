# TCP accepted-child direct-pointer ABI fix

The stream accepted-child constructor now has a direct pointer-returning ABI:

```c
void *stcp_rust_create_stream_accepted_child_ptr(void *listener_ctx);
```

Both Linux adapters use this function for TCP accept. The previous errno +
`void **out_ctx` ABI remains available for compatibility, but is no longer used
by the stream accept path. This prevents a successful return with a NULL output
context from stopping before `kernel_accept()`.
