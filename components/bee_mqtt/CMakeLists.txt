set(component_srcs "bee_mqtt.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver" "mqtt" "json" "esp_wifi"
                       REQUIRES "bee_deep_sleep")
