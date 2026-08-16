//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#include "ota_manager_internal.h"

#include "esp_log.h"
#include "mdns.h"

static const char *tag = "ota_manager_mdns";
static bool mdns_started;

esp_err_t ota_manager_mdns_start(const char *hostname, uint16_t port)
{
  if (mdns_started)
  {
    return ESP_OK;
  }

  esp_err_t err = mdns_init();
  if (err != ESP_OK)
  {
    ESP_LOGE(tag, "mdns_init failed: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGD(tag, "mDNS initialized, setting hostname: %s", hostname);

  err = mdns_hostname_set(hostname);
  if (err != ESP_OK)
  {
    ESP_LOGE(tag, "Failed to set hostname: %s", esp_err_to_name(err));
    mdns_free();
    return err;
  }

  err = mdns_instance_name_set("ESP-IDF OTA Manager");
  if (err != ESP_OK)
  {
    ESP_LOGE(tag, "Failed to set instance name: %s", esp_err_to_name(err));
    mdns_free();
    return err;
  }

  //-- Register OTA service
  err = mdns_service_add(NULL, "_esp-ota", "_tcp", port, NULL, 0);
  if (err != ESP_OK)
  {
    ESP_LOGE(tag, "mDNS OTA service registration failed: %s",
             esp_err_to_name(err));
    mdns_free();
    return err;
  }

  //-- Register HTTP service (for future web interface / discovery)
  err = mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
  if (err != ESP_OK)
  {
    ESP_LOGW(tag, "mDNS HTTP service registration failed: %s",
             esp_err_to_name(err));
    //-- Don't fail if HTTP service registration fails
  }

  //-- Register SSH service as placeholder (for future remote management)
  err = mdns_service_add(NULL, "_ssh", "_tcp", 22, NULL, 0);
  if (err != ESP_OK)
  {
    ESP_LOGW(tag, "mDNS SSH service registration failed: %s",
             esp_err_to_name(err));
    //-- Don't fail if SSH service registration fails
  }

  mdns_started = true;
  ESP_LOGI(tag, "mDNS ready: %s.local", hostname);
  ESP_LOGI(tag, "  Advertised services: _esp-ota._tcp:%u, _http._tcp, _ssh._tcp",
           (unsigned)port);
  return ESP_OK;
}

void ota_manager_mdns_stop(void)
{
  if (!mdns_started)
  {
    return;
  }

  mdns_free();
  mdns_started = false;
}
