//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t ota_manager_mdns_start(const char *hostname, uint16_t port);
void ota_manager_mdns_stop(void);
