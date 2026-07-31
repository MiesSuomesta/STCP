#!/usr/bin/env python3
"""Patch NCS 3.3.0 W5500 with low-latency IRQ polling fallback.

The shield's W5500 INT signal is not reaching the Zephyr GPIO callback on the
current nRF9151 DK mapping. Hardware TX/RX still works and Socket 0 IR contains
SENDOK/RECV. This patch keeps the normal callback path but polls S0_IR every
1 ms, so missing GPIO edges do not add hundreds of milliseconds per packet.

The patch is idempotent and repairs earlier STCP W5500 debug revisions.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import sys

MARKER = "STCP_W5500_FAST_IRQ_POLL_FALLBACK"

TX_REPLACEMENT = '''\t/* STCP_W5500_FAST_IRQ_POLL_FALLBACK:
\t * Keep the normal interrupt/semaphore path, but poll Socket 0 IR every
\t * millisecond. The current shield mapping reports SENDOK/RECV in S0_IR
\t * even though the GPIO callback does not fire.
\t */
\tk_sem_reset(&ctx->tx_sem);
\tret = w5500_command(dev, S0_CR_SEND);
\tif (ret < 0) {
\t\tLOG_ERR("SEND command failed: len=%u ret=%d", len, ret);
\t\treturn ret;
\t}

\tfor (int poll = 0; poll < 50; poll++) {
\t\tuint8_t sn_ir = 0;
\t\tint sn_ir_rc;

\t\t/* Preserve the ordinary GPIO interrupt path when it happens. */
\t\tif (k_sem_take(&ctx->tx_sem, K_MSEC(1)) == 0) {
\t\t\tLOG_DBG("TX SENDOK via GPIO: len=%u poll=%d", len, poll);
\t\t\treturn 0;
\t\t}

\t\tsn_ir_rc = w5500_spi_read(dev, W5500_S0_IR, &sn_ir, 1);
\t\tif (sn_ir_rc < 0) {
\t\t\tLOG_ERR("TX poll read failed: len=%u poll=%d rc=%d",
\t\t\t\tlen, poll, sn_ir_rc);
\t\t\treturn sn_ir_rc;
\t\t}

\t\tif ((sn_ir & S0_IR_RECV) != 0U) {
\t\t\tuint8_t clear = S0_IR_RECV;
\t\t\t(void)w5500_spi_write(dev, W5500_S0_IR, &clear, 1);
\t\t\tw5500_rx(dev);
\t\t\tLOG_DBG("RX serviced during TX poll: S0_IR=0x%02x", sn_ir);
\t\t}

\t\tif ((sn_ir & S0_IR_SENDOK) != 0U) {
\t\t\tuint8_t clear = S0_IR_SENDOK;
\t\t\tint clear_rc = w5500_spi_write(dev, W5500_S0_IR, &clear, 1);

\t\t\tLOG_DBG("TX SENDOK via fast polling: len=%u poll=%d "
\t\t\t\t "S0_IR=0x%02x clear_rc=%d",
\t\t\t\t len, poll, sn_ir, clear_rc);
\t\t\treturn clear_rc;
\t\t}
\t}

\t{
\t\tconst struct w5500_config *config = dev->config;
\t\tuint8_t ir = 0;
\t\tuint8_t sn_ir = 0;
\t\tuint8_t sn_sr = 0;
\t\tint ir_rc = w5500_spi_read(dev, W5500_IR, &ir, 1);
\t\tint sn_ir_rc = w5500_spi_read(dev, W5500_S0_IR, &sn_ir, 1);
\t\tint sn_sr_rc = w5500_spi_read(dev, W5500_S0_SR, &sn_sr, 1);
\t\tint int_level = gpio_pin_get_dt(&config->interrupt);

\t\tLOG_ERR("TX timeout after 50 ms: len=%u IR=0x%02x(rc=%d) "
\t\t\t"S0_IR=0x%02x(rc=%d) S0_SR=0x%02x(rc=%d) INT=%d",
\t\t\tlen, ir, ir_rc, sn_ir, sn_ir_rc, sn_sr, sn_sr_rc,
\t\t\tint_level);
\t}
\treturn -EIO;
'''

THREAD_REPLACEMENT = '''static void w5500_thread(void *p1, void *p2, void *p3)
{
\tARG_UNUSED(p2);
\tARG_UNUSED(p3);
\tconst struct device *dev = p1;
\tuint8_t ir;
\tstruct w5500_runtime *ctx = dev->data;
\tconst struct w5500_config *config = dev->config;
\tuint32_t poll_count = 0;
\tuint32_t irq_events = 0;
\tuint32_t rx_events = 0;
\tuint32_t tx_events = 0;

\twhile (true) {
\t\t/* STCP_W5500_FAST_IRQ_POLL_FALLBACK:
\t\t * Wake immediately on a real GPIO callback, otherwise poll S0_IR
\t\t * every millisecond. This removes the monitor-period latency from
\t\t * ARP, TCP ACK and payload reception.
\t\t */
\t\tif (k_sem_take(&ctx->int_sem, K_MSEC(1)) == 0) {
\t\t\tirq_events++;
\t\t}

\t\tif (ctx->state.is_up != true || (poll_count % 1000U) == 0U) {
\t\t\tw5500_update_link_status(dev);
\t\t}

\t\tif (w5500_spi_read(dev, W5500_S0_IR, &ir, 1) == 0 && ir != 0U) {
\t\t\t/* Writing one to an observed bit acknowledges that event. */
\t\t\t(void)w5500_spi_write(dev, W5500_S0_IR, &ir, 1);

\t\t\tif ((ir & S0_IR_SENDOK) != 0U) {
\t\t\t\tk_sem_give(&ctx->tx_sem);
\t\t\t\ttx_events++;
\t\t\t}
\t\t\tif ((ir & S0_IR_RECV) != 0U) {
\t\t\t\tw5500_rx(dev);
\t\t\t\trx_events++;
\t\t\t}

\t\t\tLOG_DBG("IRQ event: S0_IR=0x%02x INT=%d tx=%u rx=%u gpio=%u",
\t\t\t\tir, gpio_pin_get_dt(&config->interrupt),
\t\t\t\ttx_events, rx_events, irq_events);
\t\t}

\t\tpoll_count++;
\t\tif ((poll_count % 10000U) == 0U) {
\t\t\tLOG_INF("IRQ polling stats: polls=%u gpio=%u tx=%u rx=%u INT=%d",
\t\t\t\tpoll_count, irq_events, tx_events, rx_events,
\t\t\t\tgpio_pin_get_dt(&config->interrupt));
\t\t}
\t}
}
'''


def replace_tx_block(text: str) -> tuple[str, bool]:
    pattern = re.compile(
        r'\t(?:/\* STCP_W5500_[\s\S]*?\*/\n\t)?(?:k_sem_reset\(&ctx->tx_sem\);\n\t)?'
        r'(?:ret = )?w5500_command\(dev, S0_CR_SEND\);'
        r'[\s\S]*?\n\t}\n\n(?=\treturn 0;)',
        re.MULTILINE,
    )
    match = pattern.search(text)
    if not match:
        # Clean upstream block has no closing brace before return 0.
        pattern = re.compile(
            r'\tw5500_command\(dev, S0_CR_SEND\);\n'
            r'\tif \(k_sem_take\(&ctx->tx_sem, K_MSEC\(10\)\)\) \{\n'
            r'\t\treturn -EIO;\n\t\}\n',
            re.MULTILINE,
        )
        match = pattern.search(text)
    if not match:
        return text, False
    return text[:match.start()] + TX_REPLACEMENT + text[match.end():], True


def replace_thread(text: str) -> tuple[str, bool]:
    pattern = re.compile(
        r'static void w5500_thread\(void \*p1, void \*p2, void \*p3\)\n\{'
        r'[\s\S]*?\n\}\n(?=\nstatic void w5500_iface_init)',
        re.MULTILINE,
    )
    match = pattern.search(text)
    if not match:
        return text, False
    return text[:match.start()] + THREAD_REPLACEMENT + text[match.end():], True


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("driver", type=Path)
    args = parser.parse_args()
    path = args.driver

    if not path.is_file():
        print(f"[FAIL] W5500 driver not found: {path}", file=sys.stderr)
        return 2

    text = path.read_text(encoding="utf-8")
    backup = path.with_suffix(path.suffix + ".stcp-original")

    # Earlier patcher revisions accidentally wrote literal backslash-t sequences
    # into the C source. Recover automatically from the pristine backup.
    broken_patch = "\\t" in text
    if broken_patch:
        if not backup.is_file():
            print("[FAIL] Broken W5500 patch detected, but pristine backup is missing.",
                  file=sys.stderr)
            return 5
        shutil.copy2(backup, path)
        text = path.read_text(encoding="utf-8")
        print(f"[INFO] Restored pristine W5500 driver after broken patch: {backup}")

    if MARKER in text and "IRQ polling stats:" in text:
        print(f"[INFO] W5500 fast IRQ polling fallback already present: {path}")
        return 0

    if not backup.exists():
        shutil.copy2(path, backup)
        print(f"[INFO] Saved original driver: {backup}")

    text, tx_replaced = replace_tx_block(text)
    if not tx_replaced:
        print("[FAIL] W5500 TX block not found; driver left untouched.", file=sys.stderr)
        return 3

    text, thread_replaced = replace_thread(text)
    if not thread_replaced:
        print("[FAIL] W5500 thread function not found; driver left untouched.", file=sys.stderr)
        return 4

    path.write_text(text, encoding="utf-8")
    print(f"[OK] Patched W5500 low-latency TX/RX polling fallback: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
