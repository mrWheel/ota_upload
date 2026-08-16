# Changelog

## 0.1.0

### Core Features
- TCP push OTA protocol (port 3232, magic `OTAMGR01`, simple error responses)
- Direct image write using ESP-IDF OTA APIs
- Native ESP-IDF image validation (CRC, magic bytes, version)
- Automatic reboot after successful update
- NVS-based state tracking

### Network & Discovery
- mDNS hostname support (`ota-upload-example.local`)
- mDNS service advertisement:
  - `_esp-ota._tcp` — OTA firmware update service
  - `_http._tcp` — HTTP service (for future web interface and standard discovery)
  - `_ssh._tcp` — SSH service (placeholder for future remote management)
- Integration with `espressif/mdns` component
- **Important:** mDNS initialization is deferred until `IP_EVENT_STA_GOT_IP` event
  to ensure proper network stack readiness

### Wi-Fi Integration
- Optional Wi-Fi provisioning via captive portal
- Integration with `michmich/esp-idf-wifi-provisioner`
- Fallback to hardcoded credentials (configurable via `menuconfig`)

### Host Tools
- Python host package (`host_tools/`) with `idf.py ota` integration
- Automatic build artifact discovery from `project_description.json`
- Progress reporting during upload
- Chunked streaming (64KB chunks)

### Configuration
- Kconfig defaults (port, hostname, reboot behavior)
- Example partition table (4MB: nvs, otadata, phy_init, ota_0, ota_1, littlefs)
- Basic example project with Wi-Fi provisioning

### Documentation
- Protocol specification (`docs/protocol.md`)
- Security warning and v0.1.0 threat model (`docs/security.md`)
- `idf.py` integration architecture (`docs/idf_py_integration.md`)
- Public API reference (`API.md`)
