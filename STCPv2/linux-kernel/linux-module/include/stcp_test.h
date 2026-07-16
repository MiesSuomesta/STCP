#pragma once

#include <linux/types.h>

bool stcp_test_should_drop_data(void);
bool stcp_test_should_duplicate_data(void);
bool stcp_test_should_reorder_data(void);
bool stcp_test_should_drop_percent(void);
u32 stcp_test_take_delay_ms(void);
