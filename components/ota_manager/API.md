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

**Important timing requirement:** This function should be called only **after** the
WiFi interface has acquired an IP address. Use the `IP_EVENT_STA_GOT_IP` event to
ensure proper initialization order.

**Why?** mDNS requires the network interface to be fully initialized with an IP
address before it can advertise services and hostnames. Calling this function too
early will cause mDNS initialization to fail or the hostname to not be resolvable.

**Recommended pattern:**

```c
static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
  if (event_id == IP_EVENT_STA_GOT_IP)
  {
    ota_manager_config_t config = OTA_MANAGER_CONFIG_DEFAULT();
    config.hostname = "mydevice";
    ESP_ERROR_CHECK(ota_manager_start(&config));
  }
}

void app_main(void)
{
  // ... initialize WiFi provisioner ...
  ESP_ERROR_CHECK(wifi_prov_init());  // Creates event loop
  
  // Register handler AFTER event loop exists
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));
  
  ESP_ERROR_CHECK(wifi_prov_start(&config));
  // ... wait for connection ...
}
```

See the `examples/basic/main/main.c` for a complete working example.

## `ota_manager_stop()`

Reserved public API. In the current v0.1.0 skeleton graceful shutdown of the
blocking listener is not yet implemented and returns `ESP_ERR_NOT_SUPPORTED`.

## `ota_manager_is_running()`

Returns whether the receiver task has been created.
