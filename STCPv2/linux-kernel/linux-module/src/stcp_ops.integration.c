/*
 * Forward normal BSD operations to the per-socket carrier.
 */

/* bind */

ret = stcp_carrier_bind(
	ssk->carrier,
	(__force u32)sin->sin_addr.s_addr,
	(__force u16)sin->sin_port
);

if (ret)
	return ret;

return stcp_rust_bind(
	ssk->rust_ctx,
	(__force u32)sin->sin_addr.s_addr,
	(__force u16)sin->sin_port
);


/* listen */

ret = stcp_carrier_listen(
	ssk->carrier,
	backlog
);

if (ret)
	return ret;

return stcp_rust_listen(
	ssk->rust_ctx,
	backlog
);


/* connect */

ret = stcp_carrier_connect(
	ssk->carrier,
	(__force u32)sin->sin_addr.s_addr,
	(__force u16)sin->sin_port,
	flags
);

if (ret)
	return ret;

ret = stcp_rust_connect(
	ssk->rust_ctx,
	(__force u32)sin->sin_addr.s_addr,
	(__force u16)sin->sin_port,
	flags
);


/* accept, after child Rust context exists */

ret = stcp_carrier_accept(
	listener->carrier,
	child->rust_ctx,
	child,
	&child->carrier,
	flags
);

if (ret)
	return ret;

stcp_rust_set_carrier(
	child->rust_ctx,
	child->carrier
);


/* shutdown */

stcp_carrier_shutdown(
	ssk->carrier,
	how
);

stcp_rust_shutdown(
	ssk->rust_ctx,
	how
);


/* release */

stcp_stop_retransmit_work(ssk);

if (ssk->carrier) {
	stcp_carrier_destroy(ssk->carrier);
	ssk->carrier = NULL;
}

if (ssk->rust_ctx) {
	stcp_rust_set_owner(ssk->rust_ctx, NULL);
	stcp_rust_release(ssk->rust_ctx);
	ssk->rust_ctx = NULL;
}
