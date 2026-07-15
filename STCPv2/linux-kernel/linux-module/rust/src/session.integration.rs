/*
 * Every carrier::transmit() call must receive the per-context carrier
 * pointer and flags.
 *
 * Replace:
 *
 *     transmit(&shared, side, &frame, true);
 *
 * with:
 */

let carrier_ptr = {
    let inner = ctx.inner.lock();
    inner.carrier
};

transmit(
    &shared,
    side,
    carrier_ptr,
    &frame,
    0,
)?;


/*
 * Replace retransmission calls such as:
 *
 *     transmit(&shared, side, &frame, false);
 *
 * with:
 */

let carrier_ptr = {
    let inner = ctx.inner.lock();
    inner.carrier
};

transmit(
    &shared,
    side,
    carrier_ptr,
    &frame,
    0,
)?;


/*
 * In tick(), skip STCP retransmission for TCP:
 */

let carrier_ptr = {
    let inner = ctx.inner.lock();
    inner.carrier
};

if !carrier::reliability_required(carrier_ptr) {
    return Ok(true);
}


/*
 * Useful check:
 *
 * grep -R "transmit(&shared" -n rust/src
 *
 * No old four-argument transmit() calls may remain.
 */
