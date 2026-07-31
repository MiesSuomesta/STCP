# STCP modem AT shell command

Added a generic modem AT command runner to the existing STCP shell.

Examples:

```text
stcp modem at AT+CEREG?
stcp modem at AT%XMONITOR
stcp modem at AT+CESQ
stcp modem at AT+CFUN=4
stcp modem at AT%XSYSTEMMODE=1,0,0,0
stcp modem at AT+CFUN=1
```

The command:

- accepts up to 15 shell arguments and joins them with spaces,
- requires the resulting command to begin with `AT`,
- limits the command to 255 characters,
- serializes modem access through the existing AT-response mutex,
- prints both successful and error responses.
