# V6.10 – actual python/ directory replacement

The user runs:

    cd linux-module/python
    bash run_stress_suite.sh

Earlier archives updated only the repository root and `python-scripts/` while
leaving an existing `python/` directory untouched during extraction. The old
`python/run_stress_suite.sh` therefore continued to use `--pipeline 8`, which is
visible in the test output as `pipeline depth: 8` and can deadlock the echo test.

This archive now includes the exact `python/` directory used by that command.
The throughput suite uses `--pipeline 1`. The root launcher also delegates to
this canonical directory.
