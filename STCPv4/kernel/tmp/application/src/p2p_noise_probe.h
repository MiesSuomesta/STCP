#pragma once

#include <zephyr/shell/shell.h>
#include "p2p_benchmark.h"

int p2p_noise_probe_run(const struct shell *sh, const struct p2p_bench_config *cfg);
