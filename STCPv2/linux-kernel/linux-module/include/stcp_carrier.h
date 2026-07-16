#pragma once

#include <linux/types.h>

struct stcp_carrier;

enum stcp_carrier_kind {
	STCP_CARRIER_TCP = 1,
	STCP_CARRIER_UDP = 2,
};

struct stcp_carrier *stcp_carrier_create(
	enum stcp_carrier_kind kind,
	void *rust_ctx,
	void *owner
);

void stcp_carrier_destroy(
	struct stcp_carrier *carrier
);

struct stcp_carrier *stcp_carrier_create_udp_child(
	struct stcp_carrier *listener,
	void *child_rust_ctx,
	u32 peer_addr,
	u16 peer_port
);

void stcp_carrier_set_owner(
	struct stcp_carrier *carrier,
	void *owner
);

bool stcp_carrier_needs_reliability(
	const struct stcp_carrier *carrier
);

int stcp_carrier_bind(
	struct stcp_carrier *carrier,
	u32 address,
	u16 port
);

int stcp_carrier_listen(
	struct stcp_carrier *carrier,
	int backlog
);

int stcp_carrier_connect(
	struct stcp_carrier *carrier,
	u32 address,
	u16 port,
	int flags
);

int stcp_carrier_accept(
	struct stcp_carrier *listener,
	void *child_rust_ctx,
	void *child_owner,
	struct stcp_carrier **out_child,
	int flags
);

ssize_t stcp_carrier_send(
	struct stcp_carrier *carrier,
	const u8 *data,
	size_t len,
	int flags
);

void stcp_carrier_shutdown(
	struct stcp_carrier *carrier,
	int how
);
