# ota_manager

`ota_manager` is an ESP-IDF component for direct application firmware upload
from a developer computer to an ESP32 on the same local network.

It does **not** require an external firmware web server.

## Status

Version `0.1.0` is a development-oriented implementation/skeleton.

The current protocol is intentionally simple and unauthenticated. Do not expose
the OTA TCP port to untrusted networks.

## Component use

Eventually, after publication:

```bash
idf.py add-dependency "mrwheel/ota_manager^0.1.0"
```

For local development, the component can be placed in `components/ota_manager`.

## Application API

```c
ota_manager_config_t config = OTA_MANAGER_CONFIG_DEFAULT();
config.hostname = "";

ESP_ERROR_CHECK(ota_manager_start(&config));
```

The application must already have a working network connection before starting
the OTA manager.

## Dependencies

- `espressif/mdns` — mDNS hostname advertising
- `michmich/esp-idf-wifi-provisioner` — Wi-Fi provisioning (optional; fallback to hardcoded credentials)

## Partition table

An OTA-capable partition table requires `otadata` plus at least two OTA app
slots (`ota_0` and `ota_1`). See the included example.

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

The component advertises:

```text
_esp-ota._tcp
```

and sets the configured mDNS hostname.

## Security

See `docs/security.md`.

## License

GNU General Public License v3.0 or later. See `LICENSE` file.

Copyright (C) 2026 Willem Aandewiel
