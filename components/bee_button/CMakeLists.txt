set(component_srcs "bee_button.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver"
                       REQUIRES "bee_wifi" "bee_ota" "bee_nvs" "bee_mqtt" "bee_ledc")