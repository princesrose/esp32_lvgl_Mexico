#include "weatherReport.h"
LV_FONT_DECLARE(lv_font_xjz_12)
static void freeBuffer(char* buf, int bufSize){
	memset(buf, 0, bufSize);
}
static void showChongqin(char* text){
	char oledText[20];
	char *date,*data1 = NULL,*data2,*humidity;
	cJSON *root,*arrayItem,*subArray;
	cJSON *arr_item,*sub_array_item;
	cJSON *JsonDate,*JsonTemp_High,*JsonTemp_Low,*JsonHumidity, *Jsonday;
	root = cJSON_Parse(text);
	if(root!=NULL)
	{
		arrayItem = cJSON_GetObjectItem(root,"results");
		//ESP_LOGE(HTTP_TAG, "Data IS json");
		int arr_size = cJSON_GetArraySize(arrayItem);
		ESP_LOGI(HTTP_TAG, "root_arr_size: %d \n", arr_size);
		arr_item = arrayItem->child;
		for(int i = 0; i < arr_size; i ++){
			subArray = cJSON_GetObjectItem(arr_item, "daily");
			int sub_array_size = cJSON_GetArraySize(subArray);
			sub_array_item = subArray->child;
			ESP_LOGI(HTTP_TAG, "sub_arr_size: %d \n", sub_array_size);
			lv_obj_t *labeltital = lv_label_create(lv_scr_act(), NULL);
			//static lv_style_t style_label;
			//lv_style_set_text_font(&style_label, LV_STATE_DEFAULT, &lv_font_xjz_12);
			//lv_obj_add_style(labeltital, LV_LABEL_PART_MAIN, &style_label);
			lv_obj_set_style_local_text_font(labeltital, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &lv_font_xjz_12);
			lv_label_set_text(labeltital, "Mexico data:");
			for(int j = 0; j < sub_array_size; j ++){
				if(sub_array_item->type == cJSON_Object){
					lv_obj_t *labeldate = lv_label_create(lv_scr_act(), labeltital);
					lv_obj_set_pos(labeldate, j*100, 20);
					lv_obj_t *labeltemp = lv_label_create(lv_scr_act(), labeltital);
					lv_obj_set_pos(labeltemp, j*100, 40);
					lv_obj_t *labelhumi = lv_label_create(lv_scr_act(), labeltital);
					lv_obj_set_pos(labelhumi, j*100, 60);
					lv_obj_t *labelday = lv_label_create(lv_scr_act(), labeltital);
					lv_obj_set_pos(labelday, j*100, 80);

					JsonDate =  cJSON_GetObjectItem(sub_array_item, "date");
					if(cJSON_IsString(JsonDate)){
						date = JsonDate->valuestring;
						lv_label_set_text_fmt(labeldate, "时间:%s", date);
					}
					
					JsonTemp_High =  cJSON_GetObjectItem(sub_array_item, "high");
					if(cJSON_IsString(JsonTemp_High))
						data1 = JsonTemp_High->valuestring;
					
					JsonTemp_Low =  cJSON_GetObjectItem(sub_array_item, "low");
					if(cJSON_IsString(JsonTemp_Low)){
						data2 = JsonTemp_Low->valuestring;
						sprintf(oledText, "温度:%s℃-%s℃", data1, data2);
						lv_label_set_text(labeltemp, oledText);
						freeBuffer(oledText, 20);
					}
					
					JsonHumidity =  cJSON_GetObjectItem(sub_array_item, "humidity");
					if(cJSON_IsString(JsonHumidity)){
						humidity = JsonHumidity->valuestring;
						sprintf(oledText, "湿度:%s%%", humidity);
						lv_label_set_text(labelhumi, oledText);
						freeBuffer(oledText, 20);
					}

					Jsonday =  cJSON_GetObjectItem(sub_array_item, "text_day");
					if(cJSON_IsString(Jsonday)){
						data1=Jsonday->valuestring;
						sprintf(oledText, "天气:%s", data1);
						lv_label_set_text(labelday, oledText);
						freeBuffer(oledText, 20);
					}
				}
				sub_array_item = sub_array_item->next;
			}
			arr_item = arr_item -> next;
		}
		ESP_LOGI(HTTP_TAG, "Finish");
	}
	cJSON_Delete(root);
}
static void cJSON_parse_task(char* text){
	showChongqin(text);
}

/** HTTP functions **/
static void http_client_task(void *pvParameters){
	char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request
    int content_length = 0;
	static const char *URL = "http://"HOST"/v3/weather/daily.json?"	\
							 "key="UserKey"&location="Location		\
							 "&language="Language					\
							 "&unit=c&start="Start"&days="Days;
    esp_http_client_config_t config = {
        .url = URL,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET Request
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(HTTP_TAG, "HTTP client fetch headers failed");
        } else {
			int data_read = 0;
			
				data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            	if (data_read >= 0) {
            		ESP_LOGI(HTTP_TAG, "HTTP GET Status = %d, content_length = %lld",
                	esp_http_client_get_status_code(client),
                	esp_http_client_get_content_length(client));
                	printf("data:%s\n", output_buffer);
					cJSON_parse_task(output_buffer);
					//vTaskDelay(pdMS_TO_TICKS(30ULL * 60 * 1000));//三十分钟获取一次
					//vTaskDelay(pdMS_TO_TICKS(30ULL * 60 * 1000));//三十分钟获取一次
            	} else {
                	ESP_LOGE(HTTP_TAG, "Failed to read response");
            	}
				freeBuffer(output_buffer, MAX_HTTP_OUTPUT_BUFFER);
			
        }
    }
    esp_http_client_close(client);
	vTaskDelete(NULL);
}
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if(event_base == WIFI_EVENT){
		wifi_event_sta_connected_t *wifi_sta_event = (wifi_event_sta_connected_t*) event_data;
		switch(event_id){
			case WIFI_EVENT_STA_START:
				ESP_LOGI(WiFi_TAG, "connect to AP:%s ", WiFi_STA_SSID);
				esp_wifi_connect();
				break;
			case WIFI_EVENT_STA_CONNECTED:
				ESP_LOGI(WiFi_TAG, "connected");
				WiFi_retry_num = 0;
				gl_sta_connected = true;
				wifi_sta_event = (wifi_event_sta_connected_t*) event_data;
				memcpy(gl_sta_bssid, wifi_sta_event->bssid, 6);
				memcpy(gl_sta_ssid, wifi_sta_event->ssid, wifi_sta_event->ssid_len);
				gl_sta_ssid_len = wifi_sta_event->ssid_len;
				break;
			case WIFI_EVENT_STA_DISCONNECTED:
				gl_sta_connected = false;
				WiFi_retry_num ++;
				if(WiFi_retry_num > WiFi_MAX_RETRY){
					ESP_LOGI(WiFi_TAG, "retry to connect to AP:%s ", WiFi_STA_SSID);
					esp_wifi_connect();
					xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
				}else{
					xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
				}
				break;
			default:
				break;
		}
	}else if(event_base ==  IP_EVENT){
		ip_event_got_ip_t* ip_event = (ip_event_got_ip_t*) event_data;
		if(event_id == IP_EVENT_STA_GOT_IP){
			ESP_LOGI(WiFi_TAG, "got ip:" IPSTR, IP2STR(&ip_event->ip_info.ip));
			xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
			WiFi_retry_num = 0;
			gl_got_ip = true;
			//start http  task
			xTaskCreate(http_client_task, "http_client", 5120*2, NULL, 3, NULL);
		}
	}
	return;
}

static void initialise_wifi(void)
{
	wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();		//初始化并注册事件至event loop中
    assert(sta_netif);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	
	wifi_config_t wifi_config= {
        .sta = {
            .ssid = WiFi_STA_SSID,
			.password = WiFi_STA_PASSWORD,
			.bssid_set = 0,
		}
	};
	
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
	ESP_LOGI(WiFi_TAG, "wifi_init_sta finished.");
}

void xjz_wifi_begin(void *pvParameter)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	initialise_wifi();
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(30 * 60 * 1000));//三十分钟获取一次
		xTaskCreate(http_client_task, "http_client", 5120*2, NULL, 3, NULL);
    }
    vTaskDelete(NULL);      // 删除任务
}
