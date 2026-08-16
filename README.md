# ota_manager development project

This repository is a complete ESP-IDF development project for the `ota_manager`
component and its `idf.py ota` host integration.

## What is included

- `components/ota_manager/` — publishable ESP-IDF component
- `main/` — runnable development/demo application
- `examples/basic/` — self-contained Registry example
- `host_tools/` — Python package that adds `idf.py ota`
- `partitions.csv` — OTA-capable partition table
- `projectPrompt.md` — original project specification

## Important

This is a functional project skeleton, not a claim that v0.1.0 is production-secure.
The initial OTA protocol is deliberately simple and unauthenticated. Use it only on
a trusted development LAN.

The first flash must be done over USB.

## Build the development project

### First flash (USB)

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/cu.YOUR_PORT flash monitor
```

### Wi-Fi setup

**By default:** The device starts Wi-Fi provisioning (captive portal):
- Look for the provisioning AP in your Wi-Fi networks
- Connect and open the captive portal to configure Wi-Fi
- Device reboots and connects automatically

**Optional:** To use hardcoded credentials instead:

```bash
idf.py menuconfig
# Example configuration → Use Wi-Fi provisioning → disable
# Then set: Wi-Fi SSID / Wi-Fi password
idf.py build
idf.py -p /dev/cu.YOUR_PORT flash monitor
```

## Install the host-side `idf.py ota` extension

Install it into the same Python environment used by ESP-IDF:

```bash
python -m pip install -e ./host_tools
```

Then:

```bash
idf.py --help
idf.py ota --host ota-manager.local
```

`idf.py ota` locates the application binary from ESP-IDF build metadata.

## Registry publication

The publishable component is:

```text
components/ota_manager/
```

Before publication, replace placeholder repository/maintainer metadata in
`components/ota_manager/idf_component.yml`.

See `components/ota_manager/README.md`.
