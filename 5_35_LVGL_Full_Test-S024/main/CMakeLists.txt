set(SOURCES freertosTest.c weatherReport.c _wepicture_alpha_180x180.c main.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
                    REQUIRES lvgl_esp32_drivers 
					lvgl_touch 
					lvgl_tft 
					lvgl 
					lv_examples 
					esp_http_client 
					json
					esp_wifi
					nvs_flash
					protocol_examples_common
					esp-tls
					esp_driver_gptimer)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
