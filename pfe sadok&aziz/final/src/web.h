#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"

/* Déclarations des fonctions handler et des structures URI handler */
esp_err_t get_handler(httpd_req_t *req);

/* Déclaration de la fonction start_webserver */
httpd_handle_t start_webserver(void);

/* Déclaration de la fonction stop_webserver */
void stop_webserver(httpd_handle_t server);

/* Handler for GET request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Ouvrir le fichier HTML stocké dans SPIFFS */
    FILE* file = fopen("/spiffs/index.html", "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    /* Lire le contenu du fichier et l'envoyer comme réponse HTTP */
    char line[128];
    while (fgets(line, sizeof(line), file) != NULL) {
        httpd_resp_sendstr_chunk(req, line);
    }
    httpd_resp_sendstr_chunk(req, NULL); // Fin de la réponse

    fclose(file);
    return ESP_OK;
}

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    

    /* Start the httpd server */
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_uri_t uri_get = {
            .uri      = "/uri",
            .method   = HTTP_GET,
            .handler  = get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_get);
    }
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}

/* Task to start the web server */
void webserver_task(void *pvParameters) {
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize SPIFFS */
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        vTaskDelete(NULL);
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    /* Start the web server */
    httpd_handle_t server = start_webserver();
    if (server == NULL) {
        ESP_LOGE(TAG, "Failed to start server");
        vTaskDelete(NULL);
        return;
    } else {
        ESP_LOGI(TAG, "Server started successfully");
    }

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    stop_webserver(server);
    vTaskDelete(NULL);
}

