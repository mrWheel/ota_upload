idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event esp_netif ota_manager michmich__esp-idf-wifi-provisioner
)
