//-- SPDX-License-Identifier: GPL-3.0-or-later
//-- Copyright (C) 2026 Willem Aandewiel

#include "ota_upload.h"
#include "ota_upload_internal.h"

#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>

#include "esp_app_format.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define OTA_UPLOAD_MAGIC "OTAMGR01"
#define OTA_UPLOAD_MAGIC_LEN 8
#define OTA_UPLOAD_BUFFER_SIZE 4096

typedef struct __attribute__((packed))
{
  char magic[OTA_UPLOAD_MAGIC_LEN];
  uint32_t image_size_be;
} ota_upload_wire_header_t;

static const char *tag = "ota_upload";
static TaskHandle_t ota_task_handle;
static ota_upload_config_t active_config;
static char active_hostname[64];

static uint32_t read_u32_be(uint32_t value)
{
  const uint8_t *p = (const uint8_t *)&value;
  return ((uint32_t)p[0] << 24) |
         ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) |
         (uint32_t)p[3];
}

static esp_err_t recv_exact(int sock, void *buffer, size_t length)
{
  uint8_t *out = buffer;
  size_t received = 0;

  while (received < length)
  {
    int result = recv(sock, out + received, length - received, 0);
    if (result <= 0)
    {
      return ESP_FAIL;
    }
    received += (size_t)result;
  }

  return ESP_OK;
}

static void send_status(int sock, const char *status)
{
  send(sock, status, strlen(status), 0);
}

static esp_err_t handle_upload(int client_sock)
{
  ota_upload_wire_header_t header;

  if (recv_exact(client_sock, &header, sizeof(header)) != ESP_OK)
  {
    send_status(client_sock, "ERR header\n");
    return ESP_FAIL;
  }

  if (memcmp(header.magic, OTA_UPLOAD_MAGIC, OTA_UPLOAD_MAGIC_LEN) != 0)
  {
    send_status(client_sock, "ERR protocol\n");
    return ESP_ERR_INVALID_ARG;
  }

  uint32_t image_size = read_u32_be(header.image_size_be);
  if (image_size < sizeof(esp_image_header_t))
  {
    send_status(client_sock, "ERR size\n");
    return ESP_ERR_INVALID_SIZE;
  }

  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  const esp_partition_t *update_partition =
      esp_ota_get_next_update_partition(NULL);

  if (update_partition == NULL ||
      update_partition == running_partition ||
      image_size > update_partition->size)
  {
    send_status(client_sock, "ERR partition\n");
    return ESP_ERR_INVALID_SIZE;
  }

  ESP_LOGI(tag, "Receiving %" PRIu32 " bytes into %s",
           image_size, update_partition->label);

  esp_ota_handle_t ota_handle = 0;
  esp_err_t err = ESP_OK;

  if (active_config.prepare_cb != NULL)
  {
    ESP_LOGI(tag, "Preparing application for OTA");
    err = active_config.prepare_cb(active_config.prepare_ctx);
    if (err != ESP_OK)
    {
      ESP_LOGE(tag, "OTA preparation failed: %s", esp_err_to_name(err));
      send_status(client_sock, "ERR prepare\n");
      return err;
    }

    ESP_LOGI(tag, "Application is ready for OTA");
  }

  err = esp_ota_begin(update_partition, image_size, &ota_handle);
  if (err != ESP_OK)
  {
    send_status(client_sock, "ERR ota_begin\n");
    return err;
  }

  uint8_t buffer[OTA_UPLOAD_BUFFER_SIZE];
  uint32_t total_received = 0;

  while (total_received < image_size)
  {
    size_t wanted = MIN(sizeof(buffer), image_size - total_received);
    int received = recv(client_sock, buffer, wanted, 0);

    if (received <= 0)
    {
      ESP_LOGE(tag, "Transfer interrupted after %" PRIu32 " bytes",
               total_received);
      esp_ota_abort(ota_handle);
      send_status(client_sock, "ERR transfer\n");
      return ESP_FAIL;
    }

    err = esp_ota_write(ota_handle, buffer, received);
    if (err != ESP_OK)
    {
      esp_ota_abort(ota_handle);
      send_status(client_sock, "ERR write\n");
      return err;
    }

    total_received += (uint32_t)received;
    ESP_LOGD(tag, "Received %" PRIu32 "/%" PRIu32,
             total_received, image_size);
  }

  err = esp_ota_end(ota_handle);
  if (err != ESP_OK)
  {
    send_status(client_sock, "ERR validation\n");
    return err;
  }

  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK)
  {
    send_status(client_sock, "ERR boot_partition\n");
    return err;
  }

  send_status(client_sock, "OK rebooting\n");
  ESP_LOGI(tag, "OTA successful; boot partition set to %s",
           update_partition->label);

  if (active_config.reboot_after_update)
  {
    vTaskDelay(pdMS_TO_TICKS(250));
    esp_restart();
  }

  return ESP_OK;
}

static void ota_upload_task(void *arg)
{
  int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (listen_sock < 0)
  {
    ESP_LOGE(tag, "Unable to create socket: errno %d", errno);
    ota_task_handle = NULL;
    vTaskDelete(NULL);
    return;
  }

  int reuse = 1;
  setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  struct sockaddr_in address = {
    .sin_family = AF_INET,
    .sin_port = htons(active_config.port),
    .sin_addr.s_addr = htonl(INADDR_ANY),
  };

  if (bind(listen_sock, (struct sockaddr *)&address, sizeof(address)) != 0)
  {
    ESP_LOGE(tag, "Socket bind failed: errno %d", errno);
    close(listen_sock);
    ota_task_handle = NULL;
    vTaskDelete(NULL);
    return;
  }

  if (listen(listen_sock, 1) != 0)
  {
    ESP_LOGE(tag, "Socket listen failed: errno %d", errno);
    close(listen_sock);
    ota_task_handle = NULL;
    vTaskDelete(NULL);
    return;
  }

  ESP_LOGI(tag, "Listening for OTA uploads on TCP port %u",
           (unsigned)active_config.port);

  while (true)
  {
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);
    int client_sock = accept(listen_sock,
                             (struct sockaddr *)&source_addr,
                             &addr_len);

    if (client_sock < 0)
    {
      ESP_LOGW(tag, "accept failed: errno %d", errno);
      continue;
    }

    ESP_LOGI(tag, "OTA client connected");
    handle_upload(client_sock);
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
  }
}

esp_err_t ota_upload_start(const ota_upload_config_t *config)
{
  if (config == NULL || config->hostname == NULL || config->port == 0)
  {
    return ESP_ERR_INVALID_ARG;
  }

  if (ota_task_handle != NULL)
  {
    return ESP_ERR_INVALID_STATE;
  }

  active_config = *config;
  strlcpy(active_hostname, config->hostname, sizeof(active_hostname));
  active_config.hostname = active_hostname;

  if (active_config.enable_mdns)
  {
    esp_err_t err = ota_upload_mdns_start(active_hostname,
                                           active_config.port);
    if (err != ESP_OK)
    {
      return err;
    }
  }

  BaseType_t result = xTaskCreate(
      ota_upload_task,
      "ota_upload",
      CONFIG_OTA_UPLOAD_TASK_STACK_SIZE,
      NULL,
      CONFIG_OTA_UPLOAD_TASK_PRIORITY,
      &ota_task_handle);

  if (result != pdPASS)
  {
    ota_upload_mdns_stop();
    ota_task_handle = NULL;
    return ESP_ERR_NO_MEM;
  }

  return ESP_OK;
}

esp_err_t ota_upload_stop(void)
{
  if (ota_task_handle == NULL)
  {
    return ESP_ERR_INVALID_STATE;
  }

  /*
  * v1.0.0: stopping a task blocked in accept() requires retaining
   * the listening socket and shutting it down from here. Keep the public API
   * now, but report unsupported until graceful socket shutdown is completed.
   */
  return ESP_ERR_NOT_SUPPORTED;
}

bool ota_upload_is_running(void)
{
  return ota_task_handle != NULL;
}
