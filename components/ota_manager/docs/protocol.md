# OTA wire protocol — v0.1

Transport: TCP.

Default port: `3232`.

mDNS service: `_esp-ota._tcp`.

## Request

The host connects and sends a 12-byte header:

| Offset | Size | Description |
|---:|---:|---|
| 0 | 8 | ASCII magic `OTAMGR01` |
| 8 | 4 | firmware size, unsigned 32-bit big-endian |

Immediately after the header, the host sends exactly `firmware size` bytes.

The ESP32 writes chunks directly with the native ESP-IDF OTA API. The complete
image is not buffered in RAM.

## Response

The device returns one newline-terminated ASCII status line.

Success:

```text
OK rebooting
```

Possible errors include:

```text
ERR header
ERR protocol
ERR size
ERR partition
ERR ota_begin
ERR transfer
ERR write
ERR validation
ERR boot_partition
```

This protocol is deliberately minimal for v0.1. Future versions should add
version negotiation and authentication without silently changing this wire
format.
