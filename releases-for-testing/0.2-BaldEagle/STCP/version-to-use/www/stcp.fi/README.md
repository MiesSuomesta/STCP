# STCP.fi pristine site

This tree is deliberately independent of the old production site.

Build without benchmark data:
    python3 tools/site-generator/build.py --source . --output /tmp/stcp.fi-stage

Build a publishable TESTED-OK site:
    python3 tools/site-generator/build.py \
      --source . \
      --output /tmp/stcp.fi-stage \
      --quality-gate /path/to/quality-gate.json \
      --performance /path/to/performance.json

Publish only after validating the stage:
    tools/site-generator/publish.sh /tmp/stcp.fi-stage

Rules:
- Never copy the existing production site into the build.
- Never hand-code benchmark numbers into HTML.
- Failed quality-gate data is rejected.
- Missing compression-level measurements remain visibly empty.
- Production replacement uses same-filesystem renames only after the slow copy to uus-stcp.fi.
