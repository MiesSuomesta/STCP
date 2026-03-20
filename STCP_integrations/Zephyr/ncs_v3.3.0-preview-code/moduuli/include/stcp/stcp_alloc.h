#pragma once

void *stcp_alloc(size_t n);
void stcp_free(void *p);

// Rust käyttöön!
void* stcp_rust_kernel_socket_create(int fd);
void  stcp_rust_kernel_socket_destroy(void *p);