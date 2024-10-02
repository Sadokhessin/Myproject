#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>
//#include <wifi_prov.h>
const char *CONFIG_BROKER_URI = "mqtts://192.168.1.14:8883";
esp_mqtt_client_handle_t client;

extern const uint8_t mqtt_eclipseprojects_io_pem_start[] asm("_binary_cert_pem_start");
extern const uint8_t mqtt_eclipseprojects_io_pem_end[] asm("_binary_cert_pem_end");

static void mqtt_app_start(void);
void vTestCode(void *pvParameters);
void vCreateTestFunction(void);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = CONFIG_BROKER_URI,
            .verification.certificate = (const char *)mqtt_eclipseprojects_io_pem_start
        },
        .credentials = {
            .username = "master:mqtt1",
            .client_id = "cl1",
            .authentication.password = "gw8X8obQalcZWfci91AvpKMLfesjTenl"
        },
        .session = {
            .protocol_ver = MQTT_PROTOCOL_V_3_1_1
        }
    };

    mqtt_cfg.broker.verification.skip_cert_common_name_check = true;
    //ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    int msg_id = 0;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "master/cl1/attribute/brightness/3oFIIBqTyKHvKVh9Z2vd3N", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
            vCreateTestFunction();
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
        case MQTT_EVENT_PUBLISHED:

        case MQTT_EVENT_DATA:
        case MQTT_EVENT_ERROR:
            // Gérer les autres événements MQTT si nécessaire
            break;
        default:
            break;
    }
}
const char *str = "master/cl1/writeattributevalue/brightness/3oFIIBqTyKHvKVh9Z2vd3N";
// Tâches et fonctions utilitaires
void vTestCode(void *pvParameters) {
    char buffer[4];
    char i = 0;

    for (;;) {
        snprintf(buffer, sizeof(buffer), "%d", i++);
        esp_mqtt_client_publish(client,str, buffer, strlen(buffer), 0, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (i > 100)
            i = 0;
    }
}
void vCreateTestFunction(void) {
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    ESP_LOGI(TAG, "vCreateTestFunction");
    xTaskCreate(vTestCode, "TestFCT", 8192, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
}


