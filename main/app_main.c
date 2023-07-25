/* tls-mutual-auth example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "esp_sntp.h"
#include "esp_netif_sntp.h"

#include "esp_log.h"

int aws_iot_demo_main( int argc, char ** argv );

static const char *TAG = "MQTT_EXAMPLE";

uint32_t uxMsgIdx = 0;

#include "cJSON.h"

char cMQTT_Str[256];
uint32_t uxMQTT_StrSize;

void make_json_format(void)
{
	cJSON *root;
    char cTime[50];
    time_t now = 0;
    struct tm *newtime;

    time(&now);
    newtime = localtime(&now);
    
    sprintf(cTime,"%04d/%02d%02d/%02d%02d.%02d", newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,newtime->tm_min,newtime->tm_sec);

	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "DeviceName", "ESP32S3"); 
    cJSON_AddStringToObject(root, "DeviceTime", cTime); 
    cJSON_AddNumberToObject(root, "MsgIdx",  uxMsgIdx++);
    cJSON_AddNumberToObject(root, "Latitude",  37.4892633);
    cJSON_AddNumberToObject(root, "Longitude", 127.0075450);    

	char *my_json_string = cJSON_Print(root);
    strcpy( cMQTT_Str, my_json_string );
    uxMQTT_StrSize = strlen( cMQTT_Str );
    
	ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);
	cJSON_Delete(root);
}

#if 0
static void print_servers(void)
{
    ESP_LOGI(TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (esp_sntp_getservername(i)){
            ESP_LOGI(TAG, "server %d: %s", i, esp_sntp_getservername(i));
        } else {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = esp_sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                ESP_LOGI(TAG, "server %d: %s", i, buff);
        }
    }
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

#define CONFIG_SNTP_TIME_SERVER "pool.ntp.org"
static void obtain_time(void)
{
    /**
     * NTP server address could be acquired via DHCP,
     * see following menuconfig options:
     * 'LWIP_DHCP_GET_NTP_SRV' - enable STNP over DHCP
     * 'LWIP_SNTP_DEBUG' - enable debugging messages
     *
     * NOTE: This call should be made BEFORE esp acquires IP address from DHCP,
     * otherwise NTP option would be rejected by default.
     */
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
    config.start = false;                       // start SNTP service explicitly (after connecting)
    config.server_from_dhcp = true;             // accept NTP offers from DHCP server, if any (need to enable *before* connecting)
    config.renew_servers_after_new_IP = true;   // let esp-netif update configured SNTP server(s) after receiving DHCP lease
    config.index_of_first_server = 1;           // updates from server num 1, leaving server 0 (from DHCP) intact
    // configure the event on which we renew servers

    config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;

    config.sync_cb = time_sync_notification_cb; // only if we need the notification function
    esp_netif_sntp_init(&config);


    ESP_LOGI(TAG, "Starting SNTP");
    esp_netif_sntp_start();

    //print_servers();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    setenv("TZ", "KST-9", 1);
    tzset();

    esp_netif_sntp_deinit();
}

/*
 * Prototypes for the demos that can be started from this project.  Note the
 * MQTT demo is not actually started until the network is already.
 */

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %"PRIu32" bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    obtain_time();

    aws_iot_demo_main(0,NULL);

#if 0
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        make_json_format();
    }
#endif
}
