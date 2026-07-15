/*
 * Add this field to ContextInner:
 */

pub carrier: usize,

/*
 * Initialize it in every ContextInner constructor:
 *
 * - StcpContext::new(...)
 * - StcpContext::connected_child(...)
 * - any other ContextInner literal
 */

carrier: 0,

/*
 * Add this helper:
 */

pub fn set_carrier(
    ctx: &StcpContext,
    carrier: usize,
) {
    ctx.inner.lock().carrier = carrier;
}
