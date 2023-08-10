set(component_srcs "bee_wifi.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver" "wifi_provisioning" "esp_wifi"
                       REQUIRES "bee_nvs")
