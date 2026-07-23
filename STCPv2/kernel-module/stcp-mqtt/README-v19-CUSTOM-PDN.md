# v19 custom PDN socket binding

The modem first attaches using the default CID 0 context. The application then:

1. creates a separate PDP context,
2. configures APN `APN internet`,
3. activates the context,
4. resolves the modem PDN ID with `lte_lc_pdn_id_get(cid)`, and
5. applies `SO_BINDTOPDN` to each TCP benchmark socket before `connect()`.

Shell diagnostics:

```text
stcp modem contexts
stcp modem health
stcp bench upload
```

Expected transport log:

```text
Custom PDN active: cid=1 pdn_id=1 apn='APN internet'
Socket 3 bound to custom PDN: cid=1 pdn_id=1
```
