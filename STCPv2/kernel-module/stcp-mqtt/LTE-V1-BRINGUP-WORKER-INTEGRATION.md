# LTE v1 bring-up and reconnect worker integration

This change keeps the existing STCPv2 benchmark/MQTT code and adds only the
LTE lifecycle pieces proven useful in STCPv1:

- explicit modem library initialization and LTE handler registration
- registration, PDN and real IP-address readiness checks
- dedicated LTE reconnect work queue
- duplicate reconnect suppression
- offline/reconnect recovery sequence
- bounded exponential retry delay
- reconnect scheduling after registration or PDN loss

Important corrections compared with the v1 implementation:

- the reconnect work item always uses the reconnect handler
- PDN activation does not imply that an IP address exists
- network readiness is signalled only after `AT+CGPADDR` reports an address
- initial LTE searching does not trigger a reconnect loop

New public calls:

```c
int stcp_lte_transport_recover(void);
int stcp_lte_transport_schedule_reconnect(void);
```
