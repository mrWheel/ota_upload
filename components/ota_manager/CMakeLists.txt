idf_component_register(
    SRCS
        "ota_manager.c"
        "ota_manager_mdns.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "private_include"
    REQUIRES app_update esp_netif
    PRIV_REQUIRES espressif__mdns
)
