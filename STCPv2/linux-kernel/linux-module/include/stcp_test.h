#pragma once

#include <linux/types.h>

bool stcp_test_should_drop_data(void);
bool stcp_test_should_duplicate_data(void);
bool stcp_test_should_reorder_data(void);
