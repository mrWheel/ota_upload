//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"

#include "ota_manager.h"

#if CONFIG_EXAMPLE_USE_WIFI_PROVISIONING
#include "wifi_provisioner.h"
#endif

static const char *tag = "ota_example";

#if CONFIG_EXAMPLE_USE_WIFI_PROVISIONING
static esp_err_t wifi_start(void)
{
  wifi_prov_config_t config = WIFI_PROV_DEFAULT_CONFIG();

  ESP_LOGI(tag, "Starting Wi-Fi provisioning...");
  ESP_ERROR_CHECK(wifi_prov_start(&config));

  ESP_LOGI(tag, "Waiting for Wi-Fi connection...");
  ESP_ERROR_CHECK(wifi_prov_wait_for_connection(portMAX_DELAY));

  ESP_LOGI(tag, "Wi-Fi connected");
  return ESP_OK;
}
#else
static esp_err_t wifi_start(void)
{
  ESP_ERROR_CHECK(wifi_prov_init());

  wifi_prov_config_t config = WIFI_PROV_DEFAULT_CONFIG();
  config.portal_timeout = 0;  //-- Disable portal, use hardcoded credentials

  //-- Set hardcoded credentials manually
  wifi_config_t wifi_config = { 0 };
  strlcpy((char *)wifi_config.sta.ssid, CONFIG_EXAMPLE_WIFI_SSID,
          sizeof(wifi_config.sta.ssid));
  strlcpy((char *)wifi_config.sta.password, CONFIG_EXAMPLE_WIFI_PASSWORD,
          sizeof(wifi_config.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  ESP_LOGI(tag, "Attempting connection with hardcoded credentials...");
  ESP_ERROR_CHECK(wifi_prov_wait_for_connection(portMAX_DELAY));

  ESP_LOGI(tag, "Wi-Fi connected");
  return ESP_OK;
}
#endif

void app_main(void)
{
  ESP_ERROR_CHECK(wifi_start());

  ota_manager_config_t ota_config = OTA_MANAGER_CONFIG_DEFAULT();
  ota_config.hostname = "ota-manager-example";

  ESP_ERROR_CHECK(ota_manager_start(&ota_config));

  ESP_LOGI(tag, "OTA manager ready at ota-manager-example.local:%u",
           (unsigned)ota_config.port);
}
