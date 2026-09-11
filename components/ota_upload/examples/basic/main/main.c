//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "freertos/FreeRTOS.h"

#include "ota_upload.h"
#include "wifi_provisioner.h"

static const char *tag = "ota_example";
static bool ota_upload_started = false;

static esp_err_t example_prepare_for_ota(void *ctx)
{
  ESP_LOGI(tag, "OTA prepare callback invoked");
  return ESP_OK;
}

//-- Start OTA upload when IP address is acquired
static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
  ESP_LOGD(tag, "IP event received: event_id=%ld", event_id);

  if (event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(tag, "IP address acquired: " IPSTR, IP2STR(&event->ip_info.ip));

    if (!ota_upload_started)
    {
      ota_upload_config_t ota_config = OTA_UPLOAD_CONFIG_DEFAULT();
      ota_config.hostname = "ota-upload-example";
      ota_config.prepare_cb = example_prepare_for_ota;

      ESP_LOGI(tag, "Starting OTA upload with hostname: %s", ota_config.hostname);
      esp_err_t err = ota_upload_start(&ota_config);
      if (err == ESP_OK)
      {
        ota_upload_started = true;
        ESP_LOGI(tag, "OTA upload ready at ota-upload-example.local:%u",
                 (unsigned)ota_config.port);
      }
      else
      {
        ESP_LOGE(tag, "Failed to start OTA upload: %s",
                 esp_err_to_name(err));
      }
    }
  }
}

static esp_err_t wifi_start(void)
{
  //-- Initialize provisioner (creates event loop)
  ESP_ERROR_CHECK(wifi_prov_init());

  //-- Register IP event handler (now that event loop exists)
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));

  wifi_prov_config_t config = WIFI_PROV_DEFAULT_CONFIG();

  #if CONFIG_EXAMPLE_USE_WIFI_PROVISIONING
  ESP_LOGI(tag, "Starting Wi-Fi with provisioning portal...");
  #else
  ESP_LOGI(tag, "Starting Wi-Fi (provisioning disabled)...");
  config.portal_timeout = CONFIG_WIFI_PROV_PORTAL_TIMEOUT;
  #endif

  ESP_ERROR_CHECK(wifi_prov_start(&config));

  ESP_LOGI(tag, "Waiting for Wi-Fi connection...");
  ESP_ERROR_CHECK(wifi_prov_wait_for_connection(portMAX_DELAY));

  ESP_LOGI(tag, "Wi-Fi connected");
  return ESP_OK;
}

void app_main(void)
{
  esp_err_t callback_result = example_prepare_for_ota(NULL);
  ESP_LOGI(tag, "OTA prepare callback test returned: %s",
           esp_err_to_name(callback_result));
  ESP_ERROR_CHECK(callback_result);

  ESP_ERROR_CHECK(wifi_start());

  //-- OTA upload will be started by the IP event handler once IP is acquired
}
