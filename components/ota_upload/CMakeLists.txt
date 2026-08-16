idf_component_register(
    SRCS
        "ota_upload.c"
        "ota_upload_mdns.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "private_include"
    REQUIRES app_update esp_netif
    PRIV_REQUIRES espressif__mdns
)
