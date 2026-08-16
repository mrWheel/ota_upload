//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  const char *hostname;
  uint16_t port;
  bool enable_mdns;
  bool reboot_after_update;
} ota_manager_config_t;

#define OTA_MANAGER_CONFIG_DEFAULT()                    \
  {                                                     \
    .hostname = "esp-ota",                              \
    .port = CONFIG_OTA_MANAGER_PORT,                    \
    .enable_mdns = CONFIG_OTA_MANAGER_ENABLE_MDNS,      \
    .reboot_after_update = CONFIG_OTA_MANAGER_REBOOT_AFTER_UPDATE \
  }

esp_err_t ota_manager_start(const ota_manager_config_t *config);
esp_err_t ota_manager_stop(void);
bool ota_manager_is_running(void);

#ifdef __cplusplus
}
#endif
