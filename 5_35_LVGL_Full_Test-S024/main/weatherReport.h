#ifndef __WEATHERREPORT__H_
#define __WEATHERREPORT__H_

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_timer.h"
#include "protocol_examples_common.h"
#include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"


#include "esp_tls.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "lvgl/lvgl.h"			// LVGL头文件
#include "lvgl_helpers.h"		// 助手 硬件驱动相关
/*WiFi config parameter*/

#define WiFi_STA_SSID "19-8-卧室"
#define WiFi_STA_PASSWORD "13618387733"
/*
#define WiFi_STA_SSID "vivox80"
#define WiFi_STA_PASSWORD "xiaojunzhuang"*/
#define WiFi_MAX_RETRY 15

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static const char *WiFi_TAG = "wifi_sta";
static uint8_t WiFi_retry_num = 0;

/* store the station info */
static bool gl_sta_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len = 0;
static bool gl_got_ip = false;
//static lv_obj_t* img = NULL;
static TimerHandle_t weather_timer;

static const char *HTTP_TAG = "httpTask";
#define MAX_HTTP_OUTPUT_BUFFER 1300
#define HOST "api.seniverse.com"
#define UserKey "S0HOCcNbHsfQKnAEl"
#define Location "9G3W82C7N13P"
#define Language "zh-Hans"
#define Start "0"
#define Days "3"

#define CONFIG_SNTP_TIME_SERVER "time.windows.com"
static const char *TIMEZONE_TAG = "MX_Time";
/* 墨西哥时区配置（自动处理夏令时）*/
static const char MX_TZ[] = "CST6CDT,M4.1.0/2,M10.5.0/2";  // 墨西哥城时区规则
// 时间同步回调函数
static void time_sync_notification(struct timeval *tv);


static void weather_timer_callback(TimerHandle_t xTimer);
static void freeBuffer(char *buf, int bufSize);
static void showChongqin(char* text);
static void showMexico(char* Text);
static void cJSON_parse_task(char* text);
static void http_client_task(void *pvParameters);
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
static void initialise_wifi(void);
void xjz_wifi_begin(void *pvParameter);
LV_IMG_DECLARE(_wepicture_alpha_180x180);

#endif