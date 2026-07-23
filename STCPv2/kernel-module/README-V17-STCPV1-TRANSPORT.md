# V17: STCPv1-style LTE transport initialization

This version ports the proven STCPv1 initialization sequence without importing
STCPv1 FSM, DNS, worker or Rust dependencies.

Sequence:

1. `nrf_modem_lib_init()`
2. register `lte_lc` event handler
3. enable default PDN context events
4. `lte_lc_connect()`
5. wait for LTE registration
6. wait for default PDN and IP readiness
7. expose LTE/PDN/IP/RRC state to the benchmark application

Important NCS 3.3 configuration:

```ini
CONFIG_NRF_MODEM_LIB=y
CONFIG_LTE_LINK_CONTROL=y
CONFIG_LTE_LC_PDN_MODULE=y
```

Do not enable legacy `CONFIG_PDN` together with
`CONFIG_LTE_LC_PDN_MODULE`; NCS 3.3 then links two implementations of the same
`pdn_*` symbols.

An additional STCPv1-style PDN context remains optional:

```ini
CONFIG_STCP_LTE_CUSTOM_PDN=y
CONFIG_STCP_LTE_APN="APN internet"
```

Normal benchmark sockets still use the modem default context unless explicitly
bound to another PDN context.
