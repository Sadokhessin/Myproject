#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/uart.h"
#include <string.h>

#define UART_NUM UART_NUM_0
#define BUF_SIZE (4000)

void read_uart_and_store_in_nvs(nvs_handle_t my_handle) {
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    if (!data) {
        printf("Memory allocation failed!\n");
        return;
    }

    // Read data from UART
    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        // Store data in NVS
        printf("Storing data in NVS... ");

        // Find the position of the underscores in the data
        char *underscore_pos1 = strchr((char *)data, '_');
        if (underscore_pos1 != NULL) {
            // Calculate the length of the first part and store it in NVS as username
            int part1_len = underscore_pos1 - (char *)data;
            esp_err_t err = nvs_set_blob(my_handle, "username", data, part1_len);
            if (err != ESP_OK) {
                printf("Failed to store username in NVS\n");
            }

            // Find the position of the second underscore
            char *underscore_pos2 = strchr(underscore_pos1 + 1, '_');
            if (underscore_pos2 != NULL) {
                // Calculate the length of the second part and store it in NVS as password
                int part2_len = underscore_pos2 - (underscore_pos1 + 1);
                err = nvs_set_blob(my_handle, "password", underscore_pos1 + 1, part2_len);
                if (err != ESP_OK) {
                    printf("Failed to store password in NVS\n");
                }

                // Similarly, find and store the other parts (salt and verifier)
                char *underscore_pos3 = strchr(underscore_pos2 + 1, '_');
                if (underscore_pos3 != NULL) {
                    int part3_len = underscore_pos3 - (underscore_pos2 + 1);
                    err = nvs_set_blob(my_handle, "salt", underscore_pos2 + 1, part3_len);
                    if (err != ESP_OK) {
                        printf("Failed to store salt in NVS\n");
                    }

                    // Remaining part is verifier
                    int part4_len = len - (underscore_pos3 + 1 - (char *)data);
                    err = nvs_set_blob(my_handle, "verifier", underscore_pos3 + 1, part4_len);
                    if (err != ESP_OK) {
                        printf("Failed to store verifier in NVS\n");
                    }
                }
            }
        }

        printf("Done\n");
        nvs_commit(my_handle);
    }



    free(data);
}


/*void check_nvs_data(nvs_handle_t my_handle) {
    // Buffer to hold the retrieved data
    uint8_t *data = NULL;
    size_t len = 0;

    // Check if the verifier is stored in NVS
    esp_err_t err = nvs_get_blob(my_handle, "verifier", NULL, &len);
    if (err == ESP_OK) {
        // Allocate memory to hold the verifier
        data = (uint8_t *)malloc(len);
        if (data) {
            // Retrieve the verifier
            err = nvs_get_blob(my_handle, "verifier", data, &len);
            if (err == ESP_OK) {
                // Print the retrieved verifier as it is stored
                printf("Verifier: ");
                for (int i = 0; i < len; i++) {
                    printf("%c", data[i]);
                }
                printf("\n");
            } else {
                printf("Failed to retrieve verifier from NVS\n");
            }
            free(data);
        } else {
            printf("Memory allocation failed\n");
        }
    } else {
        printf("Verifier not found in NVS\n");
    }

    // Now let's check for the salt
    len = 0; // Reset the length variable
    err = nvs_get_blob(my_handle, "salt", NULL, &len); // Check the length of the salt
    if (err == ESP_OK) {
        data = (uint8_t *)malloc(len); // Allocate memory to hold the salt
        if (data) {
            err = nvs_get_blob(my_handle, "salt", data, &len); // Retrieve the salt
            if (err == ESP_OK) {
                printf("Salt: ");
                for (int i = 0; i < len; i++) {
                    printf("%c", data[i]);
                }
                printf("\n");
            } else {
                printf("Failed to retrieve salt from NVS\n");
            }
            free(data);
        } else {
            printf("Memory allocation failed\n");
        }
    } else {
        printf("Salt not found in NVS\n");
    }
}*/
void uart_nvs_task(void *pvParameter) {
    nvs_handle_t my_handle;

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Open NVS
    printf("Opening NVS handle... ");
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
    printf("Done\n");

    
        read_uart_and_store_in_nvs(my_handle);
       // check_nvs_data(my_handle);
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds
    

    // Close NVS
    nvs_close(my_handle);

    vTaskDelete(NULL);
}

