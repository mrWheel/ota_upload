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

  err = mdns_hostname_set(hostname);
  if (err != ESP_OK)
  {
    mdns_free();
    return err;
  }

  err = mdns_instance_name_set("ESP-IDF OTA Manager");
  if (err != ESP_OK)
  {
    mdns_free();
    return err;
  }

  err = mdns_service_add(NULL, "_esp-ota", "_tcp", port, NULL, 0);
  if (err != ESP_OK)
  {
    ESP_LOGE(tag, "mDNS service registration failed: %s",
             esp_err_to_name(err));
    mdns_free();
    return err;
  }

  mdns_started = true;
  ESP_LOGI(tag, "Advertising %s.local _esp-ota._tcp:%u",
           hostname, (unsigned)port);
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
