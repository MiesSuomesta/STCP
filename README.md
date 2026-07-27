# STCP GitHub Actions artifact/failure-order fix

Files:

- `.github/workflows/build.yml`
- `.github/workflows/benchmark.yml`
- `.github/workflows/stcp-overnight-benchmark.yml`

Main changes:

1. Long build/benchmark steps use `continue-on-error: true` and expose their `outcome`.
2. Result discovery, summaries and artifact uploads run before the job is failed.
3. Artifact uploads use `if: always() && !cancelled()` and `continue-on-error: true`.
4. A final explicit failure step restores the correct failed job status after uploads.
5. Artifact names include `github.run_id` and `github.run_attempt`, preventing name collisions on reruns.
6. Golden tags are created only when the overnight benchmark step really succeeded.

Install from repository root:

```bash
unzip stcp-workflows-artifact-fix.zip -d /tmp/stcp-workflow-fix
cp /tmp/stcp-workflow-fix/.github/workflows/*.yml .github/workflows/
```
