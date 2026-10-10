#!/usr/bin/env python3
"""Install a conservative W5500 IRQ polling fallback for NCS 3.3.0.

This version is intentionally conservative:
- 1 ms polling, not 100 us
- never calls RX from the TX hot path
- does not touch RX until the network interface has been initialized
- preserves the normal GPIO interrupt path
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil
import sys

MARKER = "STCP_W5500_STABLE_1MS_POLLING"

TX_REPLACEMENT = r'''	/* STCP_W5500_STABLE_1MS_POLLING:
	 * The shield INT line does not reach the GPIO callback on this setup.
	 * Preserve the normal semaphore path and poll SENDOK once per millisecond.
	 */
	k_sem_reset(&ctx->tx_sem);
	ret = w5500_command(dev, S0_CR_SEND);
	if (ret < 0) {
		LOG_ERR("SEND command failed: len=%u ret=%d", len, ret);
		return ret;
	}

	for (int poll = 0; poll < 100; poll++) {
		uint8_t sn_ir = 0;
		int sn_ir_rc;

		if (k_sem_take(&ctx->tx_sem, K_MSEC(1)) == 0) {
			return 0;
		}

		sn_ir_rc = w5500_spi_read(dev, W5500_S0_IR, &sn_ir, 1);
		if (sn_ir_rc < 0) {
			LOG_ERR("TX poll read failed: len=%u poll=%d rc=%d",
				len, poll, sn_ir_rc);
			return sn_ir_rc;
		}

		if ((sn_ir & S0_IR_SENDOK) != 0U) {
			uint8_t clear = S0_IR_SENDOK;
			int clear_rc = w5500_spi_write(dev, W5500_S0_IR, &clear, 1);
			return clear_rc;
		}
	}

	{
		const struct w5500_config *config = dev->config;
		uint8_t ir = 0;
		uint8_t sn_ir = 0;
		uint8_t sn_sr = 0;
		int ir_rc = w5500_spi_read(dev, W5500_IR, &ir, 1);
		int sn_ir_rc = w5500_spi_read(dev, W5500_S0_IR, &sn_ir, 1);
		int sn_sr_rc = w5500_spi_read(dev, W5500_S0_SR, &sn_sr, 1);
		int int_level = gpio_pin_get_dt(&config->interrupt);

		LOG_ERR("TX timeout: len=%u IR=0x%02x(rc=%d) "
			"S0_IR=0x%02x(rc=%d) S0_SR=0x%02x(rc=%d) INT=%d",
			len, ir, ir_rc, sn_ir, sn_ir_rc, sn_sr, sn_sr_rc,
			int_level);
	}
	return -EIO;
'''

THREAD_REPLACEMENT = r'''static void w5500_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	const struct device *dev = p1;
	uint8_t ir;
	struct w5500_runtime *ctx = dev->data;
	const struct w5500_config *config = dev->config;
	uint32_t poll_count = 0;
	uint32_t irq_events = 0;
	uint32_t rx_events = 0;
	uint32_t tx_events = 0;

	while (true) {
		if (k_sem_take(&ctx->int_sem, K_MSEC(1)) == 0) {
			irq_events++;
		}

		/* Do not touch the network RX path before iface_init has completed. */
		if (ctx->iface == NULL) {
			continue;
		}

		if (ctx->state.is_up != true || (poll_count % 1000U) == 0U) {
			w5500_update_link_status(dev);
		}

		if (w5500_spi_read(dev, W5500_S0_IR, &ir, 1) == 0 && ir != 0U) {
			/* Ack only the events we process. */
			if ((ir & S0_IR_SENDOK) != 0U) {
				uint8_t clear = S0_IR_SENDOK;
				(void)w5500_spi_write(dev, W5500_S0_IR, &clear, 1);
				k_sem_give(&ctx->tx_sem);
				tx_events++;
			}
			if ((ir & S0_IR_RECV) != 0U) {
				uint8_t clear = S0_IR_RECV;
				(void)w5500_spi_write(dev, W5500_S0_IR, &clear, 1);
				w5500_rx(dev);
				rx_events++;
			}
		}

		poll_count++;
		if ((poll_count % 10000U) == 0U) {
			LOG_INF("IRQ polling stats: polls=%u gpio=%u tx=%u rx=%u INT=%d",
				poll_count, irq_events, tx_events, rx_events,
				gpio_pin_get_dt(&config->interrupt));
		}
	}
}
'''


def ensure_forward_declaration(text: str) -> str:
    decl = "static void w5500_rx(const struct device *dev);\n\n"
    if decl in text:
        return text
    idx = text.find("static int w5500_tx(")
    if idx < 0:
        raise RuntimeError("w5500_tx not found")
    return text[:idx] + decl + text[idx:]


def replace_tx(text: str) -> str:
    # Match upstream or any previous STCP replacement up to the final return 0.
    start = text.find("static int w5500_tx(")
    if start < 0:
        raise RuntimeError("w5500_tx not found")
    ret0 = text.find("\n\treturn 0;", start)
    if ret0 < 0:
        raise RuntimeError("w5500_tx return not found")
    send = text.find("w5500_command(dev, S0_CR_SEND);", start, ret0)
    if send < 0:
        send = text.find("ret = w5500_command(dev, S0_CR_SEND);", start, ret0)
    if send < 0:
        raise RuntimeError("W5500 TX SEND block not found")
    line_start = text.rfind("\n", start, send) + 1
    return text[:line_start] + TX_REPLACEMENT + text[ret0:]


def replace_thread(text: str) -> str:
    pat = re.compile(
        r"static void w5500_thread\(void \*p1, void \*p2, void \*p3\)\n\{[\s\S]*?\n\}\n(?=\nstatic void w5500_iface_init)",
        re.MULTILINE,
    )
    m = pat.search(text)
    if not m:
        raise RuntimeError("w5500_thread not found")
    return text[:m.start()] + THREAD_REPLACEMENT + text[m.end():]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("driver", type=Path)
    ns = ap.parse_args()
    path = ns.driver
    if not path.is_file():
        print(f"[FAIL] W5500 driver not found: {path}", file=sys.stderr)
        return 2

    backup = path.with_suffix(path.suffix + ".stcp-original")
    current = path.read_text(encoding="utf-8")

    # Always start from the pristine driver when available. This prevents old
    # experimental patch revisions from leaking into a new build.
    if backup.is_file():
        shutil.copy2(backup, path)
        current = path.read_text(encoding="utf-8")
        print(f"[INFO] Restored pristine W5500 driver: {backup}")
    else:
        shutil.copy2(path, backup)
        print(f"[INFO] Saved pristine W5500 driver: {backup}")

    try:
        current = ensure_forward_declaration(current)
        current = replace_tx(current)
        current = replace_thread(current)
    except RuntimeError as exc:
        print(f"[FAIL] {exc}; driver left pristine", file=sys.stderr)
        shutil.copy2(backup, path)
        return 3

    path.write_text(current, encoding="utf-8")
    print(f"[OK] Installed stable 1 ms W5500 polling fallback: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
