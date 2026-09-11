# ota_upload development project

This repository is a complete ESP-IDF development project for the `ota_upload`
component and its `idf.py ota` host integration.

## What is included

- `components/ota_upload/` — publishable ESP-IDF component
- `main/` — runnable development/demo application
- `examples/basic/` — self-contained Registry example
- `host_tools/` — Python package that adds `idf.py ota`
- `partitions.csv` — OTA-capable partition table
- `projectPrompt.md` — original project specification

## Important

This is a functional development project, not a claim that v1.0.0 is production-secure.
The initial OTA protocol is deliberately simple and unauthenticated. Use it only on
a trusted development LAN.

The first flash must be done over USB.

## Build the development project

### Flash size

This project uses an OTA partition table and requires a flash size of at least
**4 MB**. The project default is set to 4 MB. If your board has a larger flash,
you can select the actual size with:

```bash
idf.py menuconfig
```

Then open **Serial Flasher config** → **Flash size** and select **4 MB** or a
larger value. Save the configuration and rebuild the project:

```bash
idf.py build
```

### First flash (USB)

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/cu.YOUR_PORT flash monitor
```

### Wi-Fi setup

The root development application uses the SSID and password configured through
`CONFIG_EXAMPLE_WIFI_SSID` and `CONFIG_EXAMPLE_WIFI_PASSWORD`. It starts in
station mode and waits for that network to provide an IP address.

For the self-contained provisioning workflow, use the basic example instead:

```bash
cd components/ota_upload/examples/basic
idf.py build
idf.py flash monitor
```

The basic example uses a provisioning portal by default. On first boot, connect
to the `ESP-Provision` access point and configure the target Wi-Fi network.

To use hardcoded credentials in the basic example instead:

```bash
idf.py menuconfig
# Example configuration -> Use Wi-Fi provisioning -> disable
# Then set the Wi-Fi SSID and password
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
idf.py ota --host <hostname-or-ip>
```

`idf.py ota` requires the target hostname or IP address and locates the
application binary from ESP-IDF build metadata. Host-side mDNS discovery and
interactive device selection are not implemented yet. The ESP32 advertises
`_esp-ota._tcp` for discovery with tools such as `dns-sd`.

The OTA preparation callback can quiesce application work before flash writing
and image validation:

```c
ota_upload_config_t config = OTA_UPLOAD_CONFIG_DEFAULT();
config.hostname = "mydevice";
config.prepare_cb = application_prepare_for_ota;
config.prepare_ctx = &application_state;

ESP_ERROR_CHECK(ota_upload_start(&config));
```

## Registry publication

The publishable component is:

```text
components/ota_upload/
```

Before publication, replace placeholder repository/maintainer metadata in
`components/ota_upload/idf_component.yml`.

See `components/ota_upload/README.md`.
