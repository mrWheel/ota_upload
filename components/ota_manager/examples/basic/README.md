# ota_manager basic example

This example demonstrates direct OTA firmware upload with Wi-Fi provisioning.

## Quick start

### 1. First flash (USB)

```bash
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

Watch the serial output for the provisioning AP details.

### 2. Wi-Fi provisioning (optional)

By default, the example uses **Wi-Fi provisioning** via BLE/SoftAP (captive portal):

- Look for the provisioning AP in your Wi-Fi networks
- Connect and open the captive portal
- Select your Wi-Fi SSID and enter password
- Device will reboot and connect

To disable provisioning and use **hardcoded credentials** instead:

```bash
idf.py menuconfig
# Example configuration → Use Wi-Fi provisioning → disable
# Then set SSID and password
```

### 3. OTA upload

Install the host tools:

```bash
pip install -e ./host_tools
```

Then upload firmware:

```bash
idf.py build
idf.py ota --host ota-manager-example.local
```

The device will validate and reboot with the new firmware.

## Registry distribution

For development inside the source repository, the example manifest uses
`override_path` to the local component.

When distributed by the ESP Component Registry, adjust/use the generated
Registry example dependency as appropriate for the published namespace/version.
