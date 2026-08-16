//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "ota_manager.h"

static const char *tag = "ota_example";
static EventGroupHandle_t wifi_event_group;
static const EventBits_t wifi_connected_bit = BIT0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    ESP_LOGW(tag, "Wi-Fi disconnected; reconnecting");
    esp_wifi_connect();
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    xEventGroupSetBits(wifi_event_group, wifi_connected_bit);
  }
}

static esp_err_t wifi_start(void)
{
  wifi_event_group = xEventGroupCreate();
  if (wifi_event_group == NULL)
  {
    return ESP_ERR_NO_MEM;
  }

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                              &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                              &wifi_event_handler, NULL));

  wifi_config_t wifi_config = { 0 };
  strlcpy((char *)wifi_config.sta.ssid, CONFIG_EXAMPLE_WIFI_SSID,
          sizeof(wifi_config.sta.ssid));
  strlcpy((char *)wifi_config.sta.password, CONFIG_EXAMPLE_WIFI_PASSWORD,
          sizeof(wifi_config.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(tag, "Waiting for Wi-Fi connection...");
  xEventGroupWaitBits(wifi_event_group, wifi_connected_bit, pdFALSE, pdTRUE,
                      portMAX_DELAY);

  ESP_LOGI(tag, "Wi-Fi connected");
  return ESP_OK;
}

void app_main(void)
{
  esp_err_t err = nvs_flash_init();

  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }
  else
  {
    ESP_ERROR_CHECK(err);
  }

  ESP_ERROR_CHECK(wifi_start());

  ota_manager_config_t ota_config = OTA_MANAGER_CONFIG_DEFAULT();
  ota_config.hostname = "ota-manager";

  ESP_ERROR_CHECK(ota_manager_start(&ota_config));

  ESP_LOGI(tag, "OTA manager ready at ota-manager.local:%u",
           (unsigned)ota_config.port);
}
