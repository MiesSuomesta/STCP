# V18 default CID 0 APN

The literal APN `APN internet` is written to PDP context CID 0 before LTE attach.

```ini
CONFIG_STCP_LTE_FORCE_DEFAULT_APN=y
CONFIG_STCP_LTE_DEFAULT_APN="APN internet"
CONFIG_STCP_LTE_CUSTOM_PDN=n
```

At boot the application logs `AT+CGDCONT?` before `lte_lc_connect()`.
Note: if the operator rewrites the active context APN after attach, the post-attach
query can still show `internet`; the pre-attach log proves what the application set.
