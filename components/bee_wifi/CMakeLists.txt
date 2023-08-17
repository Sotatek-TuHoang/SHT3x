set(component_srcs)

idf_component_register(SRCS "bee_wifi.c" "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver" "wifi_provisioning" "esp_wifi"
                       REQUIRES "bee_nvs")
