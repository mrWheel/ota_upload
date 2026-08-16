# API

## `ota_manager_config_t`

Runtime configuration for the OTA service.

Fields:

- `hostname` — mDNS hostname without `.local`
- `port` — TCP listening port
- `enable_mdns` — advertise `_esp-ota._tcp`
- `reboot_after_update` — reboot after a validated image becomes bootable

## `OTA_MANAGER_CONFIG_DEFAULT()`

Creates a configuration using Kconfig defaults.

## `ota_manager_start()`

Starts mDNS (when enabled) and the OTA receiver task.

The application must have network connectivity before calling this function.

## `ota_manager_stop()`

Reserved public API. In the current v0.1.0 skeleton graceful shutdown of the
blocking listener is not yet implemented and returns `ESP_ERR_NOT_SUPPORTED`.

## `ota_manager_is_running()`

Returns whether the receiver task has been created.
