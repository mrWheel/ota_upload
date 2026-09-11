# API

## `ota_upload_prepare_cb_t`

```c
typedef esp_err_t (*ota_upload_prepare_cb_t)(void *ctx);
```

Application callback invoked before an OTA write begins. Return `ESP_OK` only
when application tasks and peripherals are ready for flash writing and image
validation. The callback runs in the OTA receiver task and is never called from
an ISR.

## `ota_upload_config_t`

Runtime configuration for the OTA service.

Fields:

- `hostname` — mDNS hostname without `.local`
- `port` — TCP listening port
- `enable_mdns` — advertise `_esp-ota._tcp`
- `reboot_after_update` — reboot after a validated image becomes bootable
- `prepare_cb` — optional callback invoked before `esp_ota_begin()`
- `prepare_ctx` — application context passed to `prepare_cb`

### OTA preparation callback

Applications can stop background tasks and quiesce peripherals before flash
writing and image validation:

```c
static esp_err_t application_prepare_for_ota(void *ctx)
{
  application_state_t *state = (application_state_t *)ctx;

  return application_stop_background_work(state);
}

ota_upload_config_t config = OTA_UPLOAD_CONFIG_DEFAULT();
config.hostname = "mydevice";
config.prepare_cb = application_prepare_for_ota;
config.prepare_ctx = &application_state;

ESP_ERROR_CHECK(ota_upload_start(&config));
```

The callback runs once in the OTA task after the request and update partition
have been validated, but before `esp_ota_begin()`. It must return `ESP_OK` only
when the application is ready for OTA. A non-`ESP_OK` result aborts the upload;
the currently running firmware remains the bootable image. The callback must
use bounded waits and must not wait for the OTA task, because it executes in
that task's context.

## `OTA_UPLOAD_CONFIG_DEFAULT()`

Creates a configuration using Kconfig defaults.

## `ota_upload_start()`

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
    ota_upload_config_t config = OTA_UPLOAD_CONFIG_DEFAULT();
    config.hostname = "mydevice";
    ESP_ERROR_CHECK(ota_upload_start(&config));
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

## `ota_upload_stop()`

Reserved public API. In the current v1.0.0 implementation graceful shutdown of the
blocking listener is not yet implemented and returns `ESP_ERR_NOT_SUPPORTED`.

## `ota_upload_is_running()`

Returns whether the receiver task has been created.
