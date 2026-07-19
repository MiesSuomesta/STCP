#include <linux/init.h>
#include <linux/module.h>
#include <linux/atomic.h>

#include "stcp_proto.h"
#include "stcp_rust_ffi.h"
#include "stcp_test.h"
#include "stcp_users.h"

static bool drop_first_data;
static bool duplicate_first_data;
static bool reorder_first_pair;
static unsigned int drop_percent;
static unsigned int delay_first_data_ms;

static atomic_t drop_budget = ATOMIC_INIT(0);
static atomic_t duplicate_budget = ATOMIC_INIT(0);
static atomic_t reorder_budget = ATOMIC_INIT(0);
static atomic_t drop_sequence = ATOMIC_INIT(0);
static atomic_t delay_budget = ATOMIC_INIT(0);

module_param(drop_first_data, bool, 0644);
MODULE_PARM_DESC(drop_first_data,
	"Drop the first UDP DATA frame to test retransmission");

module_param(duplicate_first_data, bool, 0644);
MODULE_PARM_DESC(duplicate_first_data,
	"Duplicate the first UDP DATA frame to test duplicate suppression");

module_param(reorder_first_pair, bool, 0644);
MODULE_PARM_DESC(reorder_first_pair,
	"Send the first two UDP DATA frames in reverse order");

module_param(drop_percent, uint, 0644);
MODULE_PARM_DESC(drop_percent,
	"Deterministically drop approximately N percent of UDP DATA frames");

module_param(delay_first_data_ms, uint, 0644);
MODULE_PARM_DESC(delay_first_data_ms,
	"Delay the first UDP DATA frame by N milliseconds");

bool stcp_test_active(void)
{
	return READ_ONCE(drop_first_data) ||
	       READ_ONCE(duplicate_first_data) ||
	       READ_ONCE(reorder_first_pair) ||
	       READ_ONCE(drop_percent) != 0 ||
	       READ_ONCE(delay_first_data_ms) != 0;
}

bool stcp_test_should_drop_data(void)
{
	return atomic_cmpxchg(&drop_budget, 1, 0) == 1;
}

bool stcp_test_should_duplicate_data(void)
{
	return atomic_cmpxchg(&duplicate_budget, 1, 0) == 1;
}

bool stcp_test_should_reorder_data(void)
{
	return atomic_cmpxchg(&reorder_budget, 1, 0) == 1;
}

bool stcp_test_should_drop_percent(void)
{
	unsigned int percent = READ_ONCE(drop_percent);
	unsigned int sequence;

	if (!percent)
		return false;

	if (percent > 100)
		percent = 100;

	sequence = (unsigned int)atomic_inc_return(&drop_sequence);

	/*
	 * A coprime multiplier spreads drops across the stream while keeping
	 * the test fully deterministic and reproducible.
	 */
	return ((sequence * 37u) % 100u) < percent;
}

u32 stcp_test_take_delay_ms(void)
{
	if (atomic_cmpxchg(&delay_budget, 1, 0) != 1)
		return 0;

	return READ_ONCE(delay_first_data_ms);
}

static int __init stcp_module_init(void)
{
	int ret;

	atomic_set(&drop_budget, drop_first_data ? 1 : 0);
	atomic_set(&duplicate_budget, duplicate_first_data ? 1 : 0);
	atomic_set(&reorder_budget, reorder_first_pair ? 1 : 0);
	atomic_set(&drop_sequence, 0);
	atomic_set(&delay_budget, delay_first_data_ms ? 1 : 0);

	ret = stcp_rust_init();
	if (ret)
		return ret;

	ret = stcp_rust_crypto_selftest();
	if (ret) {
		pr_err("stcp: crypto selftest failed: %d\n", ret);
		stcp_rust_exit();
		return ret;
	}

	pr_info("stcp: directional crypto selftest passed\n");

	ret = stcp_users_init();
	if (ret) {
		stcp_rust_exit();
		return ret;
	}

	ret = stcp_proto_register();
	if (ret) {
		stcp_users_exit();
		stcp_rust_exit();
		return ret;
	}

	pr_info("stcp: loopback BSD transport loaded\n");
	return 0;
}

static void __exit stcp_module_exit(void)
{
	stcp_proto_unregister();
	stcp_users_exit();
	stcp_rust_exit();
	pr_info("stcp: loopback BSD transport unloaded\n");
}

module_init(stcp_module_init);
module_exit(stcp_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STCPv2");
MODULE_DESCRIPTION("Rust-backed STCP BSD socket loopback implementation");
