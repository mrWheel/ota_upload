# ota_upload

`ota_upload` is an ESP-IDF component for direct application firmware upload
from a developer computer to an ESP32 on the same local network.

It does **not** require an external firmware web server.

## Status

Version `0.1.0` is a development-oriented implementation/skeleton.

The current protocol is intentionally simple and unauthenticated. Do not expose
the OTA TCP port to untrusted networks.

## Component use

Eventually, after publication:

```bash
idf.py add-dependency "mrwheel/ota_upload^0.1.0"
```

For local development, the component can be placed in `components/ota_upload`.

## Application API

```c
ota_upload_config_t config = OTA_UPLOAD_CONFIG_DEFAULT();
config.hostname = "mydevice";

ESP_ERROR_CHECK(ota_upload_start(&config));
```

**Important:** The OTA upload must be started **after** the WiFi network interface
has acquired an IP address. Use the `IP_EVENT_STA_GOT_IP` event to ensure proper
timing. This guarantees that mDNS initialization will succeed and the hostname
will be properly advertised on the local network.

See the example `main.c` for the recommended event handler pattern.

## Dependencies

- `espressif/mdns` — mDNS hostname advertising
- `michmich/esp-idf-wifi-provisioner` — Wi-Fi provisioning (optional; fallback to hardcoded credentials)

## Partition table

An OTA-capable partition table requires `otadata` plus at least two OTA app
slots (`ota_0` and `ota_1`). The included OTA partition table requires a flash
size of at least **4 MB**. Configure it with `idf.py menuconfig` under
**Serial Flasher config** → **Flash size**, then select **4 MB** or larger.

## Wi-Fi provisioning

The example uses **`michmich/esp-idf-wifi-provisioner`** for automatic Wi-Fi setup.

On first boot, the device advertises a provisioning AP (captive portal). Connect
your computer, select your home Wi-Fi, and the device joins the network
automatically.

Credentials are saved in NVS and reused on reboot.

Optionally: disable provisioning and use hardcoded credentials via `menuconfig`.

## Host integration

The repository contains `host_tools/`, an installable Python package that adds:

```bash
idf.py ota --host thisProject.local
```

Because third-party Registry namespaces are not automatically trusted for
component-provided `idf_ext.py` files, the host integration is supplied through
the official Python `idf_extension` entry-point mechanism.

Install:

```bash
pip install -e ./host_tools
```

See `docs/idf_py_integration.md`.

## mDNS

The component advertises the configured hostname and the following mDNS services:

| Service | Port | Purpose |
|---------|------|---------|
| `_esp-ota._tcp` | custom (default 3232) | OTA firmware update service |
| `_http._tcp` | 80 | HTTP service (for future web interface and standard discovery) |
| `_ssh._tcp` | 22 | SSH service (placeholder for future remote management) |

**Hostname resolution:**

Devices can be accessed by hostname:

```bash
ping mydevice.local
idf.py ota --host mydevice.local
```

**Service discovery:**

Use standard mDNS browsing tools:

```bash
dns-sd -B _esp-ota._tcp local    # Find OTA devices
dns-sd -B _http._tcp local       # Find HTTP services
```

**Timing requirement:**

mDNS is only initialized after the WiFi interface has acquired an IP address
(the `IP_EVENT_STA_GOT_IP` event). Attempting to resolve the hostname before
this event will fail. This is by design to ensure the network stack is fully
ready for mDNS operation.

## Security

See `docs/security.md`.

## License

GNU General Public License v3.0 or later. See `LICENSE` file.

Copyright (C) 2026 Willem Aandewiel
