# Benchmark runner reset and UART result fix

- Uses nrfutil RESET_PIN to match the physical reset button.
- Replaces long multipart JSON with a short repeated STCP_RESULT line.
- Host runner expands STCP_RESULT into schema_version 2 JSON.
- Sets Ethernet log level to warning to stop periodic W5500 INFO logs from flooding UART.
- Does not modify the external NCS W5500 driver source.
