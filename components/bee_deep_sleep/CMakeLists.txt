set(component_srcs "bee_deep_sleep.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver" "soc"
                       REQUIRES "bee_sht3x" "bee_i2c" "bee_mqtt" "bee_wifi" "bee_nvs")
