#pragma once
#include <stcp/structures.h>   // stcp_sock, stcp_from_socket
#include <linux/spinlock.h>

// Konteksti pointterin laitto / haku
void *stcp_rust_blob_get(struct socket *sock);
void  stcp_rust_blob_set(struct socket *sock, void *ctx);

// Konteksti kerneli spinlokin lukitus/vapautus
void stcp_rust_blob_lock(struct socket *sock);
void stcp_rust_blob_unlock(struct socket *sock);

