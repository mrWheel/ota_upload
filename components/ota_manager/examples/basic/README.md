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

### 3. Verify mDNS and network connectivity

Once the device is connected to Wi-Fi, verify it is discoverable:

```bash
# Ping by hostname
ping ota-manager-example.local

# Or browse mDNS services
dns-sd -B _esp-ota._tcp local
```

You should see the device advertised as "ESP-IDF OTA Manager".

### 4. OTA upload

Install the host tools:

```bash
pip install -e ./host_tools
```

Then upload firmware by hostname:

```bash
idf.py build
idf.py ota --host ota-manager-example.local
```

Or by IP address if mDNS is not working:

```bash
idf.py ota --host 192.168.x.x
```

The device will validate and reboot with the new firmware.

## Troubleshooting

### mDNS hostname not resolving

**Problem:** `ping ota-manager-example.local` fails with "nodename nor servname provided"

**Cause:** mDNS is only advertised after the WiFi interface has acquired an IP address.

**Solution:** Wait for the serial output to show "OTA manager ready" before attempting
to resolve the hostname. Check that:

1. The device shows "IP address acquired" in the logs
2. The device shows "OTA manager ready at ota-manager-example.local" 
3. Your computer can resolve `.local` domains (mDNS/Bonjour support)

On macOS/Linux, this typically works out of the box. On Windows, ensure Bonjour is installed.

### OTA upload fails after successful device discovery

If `ping` works but `idf.py ota` fails, check:

1. Both devices are on the same network segment
2. No firewall is blocking TCP port 3232
3. The serial output shows "Listening for OTA uploads on TCP port 3232"

## Registry distribution

For development inside the source repository, the example manifest uses
`override_path` to the local component.

When distributed by the ESP Component Registry, adjust/use the generated
Registry example dependency as appropriate for the published namespace/version.
