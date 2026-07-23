# v20: APN handling is fully optional

The default build does not write any APN and does not create a custom PDN:

```ini
CONFIG_STCP_LTE_FORCE_DEFAULT_APN=n
CONFIG_STCP_LTE_CUSTOM_PDN=n
```

The modem/SIM/operator default context is used as-is.

## Optional CID 0 APN override

```ini
CONFIG_STCP_LTE_FORCE_DEFAULT_APN=y
CONFIG_STCP_LTE_DEFAULT_APN="APN internet"
```

## Optional separate PDN context

```ini
CONFIG_STCP_LTE_CUSTOM_PDN=y
CONFIG_STCP_LTE_APN="APN internet"
```

When enabled, benchmark sockets are bound to the custom PDN with `SO_BINDTOPDN`.
