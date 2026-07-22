# Prototype fix v3

This version keeps the declarations in both public headers and the C translation
units that use/define them. This avoids stale out-of-tree Kbuild dependency
metadata causing implicit declarations during incremental builds.

Before building, run:

```bash
./verify-accept-fix.sh
find raspberry-kernel-module x86-kernel-module -name '*.o' -o -name '*.cmd' | xargs -r rm -f
```
