#include <wifi_prov.h>
#include "driver/gpio.h"
#include <usine.h>
#include <web.h>
#include <mqtt.h>





bool check_nvs_storage() {
    // Ouvrez l'espace de stockage NVS
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (ret == ESP_OK) {
        // Obtenez le nombre d'entrées dans l'espace de stockage NVS
        size_t used_entries;
        nvs_get_used_entry_count(nvs_handle, &used_entries);

        // Fermez l'espace de stockage NVS
        nvs_close(nvs_handle);

        // Si le nombre d'entrées est égal à zéro, retournez true
        return (used_entries ==0);
    } else {
        printf("Impossible d'ouvrir l'espace de stockage NVS (%s).\n", esp_err_to_name(ret));
        return false;
    }
}

















void app_main(void)
{

    #define reset_wifi 27
  
    /* Initialize NVS partition */
   esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated
         // and needs to be erased 
        ESP_ERROR_CHECK(nvs_flash_erase());

        ESP_ERROR_CHECK(nvs_flash_init());
    } 
   
    while (check_nvs_storage()) {

        ESP_LOGI("Main", "mode usine en cours!");
         xTaskCreate(uart_nvs_task, "uart_nvs_task", 4096, NULL, 5, NULL);

    }


    init_et_lire_nvs() ;


    esp_rom_gpio_pad_select_gpio(reset_wifi);
    gpio_set_direction(reset_wifi, GPIO_MODE_INPUT);
    gpio_set_pull_mode(reset_wifi, GPIO_PULLUP_ONLY);

 














    

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
      
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
#ifdef CONFIG_EXAMPLE_RESET_PROVISIONED
    wifi_prov_mgr_reset_provisioning();
#else
    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

#endif
    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        


        


        wifi_prov_security_t security = WIFI_PROV_SECURITY_2;
        /* The username must be the same one, which has been used in the generation of salt and verifier */

        const char *username  = EXAMPLE_PROV_SEC2_USERNAME;
        const char *pop = EXAMPLE_PROV_SEC2_PWD;

        wifi_prov_security2_params_t sec2_params = {};
      
        ESP_ERROR_CHECK(example_get_sec2_salt(&sec2_params.salt, &sec2_params.salt_len));
        ESP_ERROR_CHECK(example_get_sec2_verifier(&sec2_params.verifier, &sec2_params.verifier_len));

        wifi_prov_security2_params_t *sec_params = &sec2_params;

        const char *service_key = NULL;


        wifi_prov_mgr_disable_auto_stop(1000);

        /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *) sec_params, service_name, service_key));

        wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);

        wifi_prov_print_qr(service_name, username, pop, PROV_TRANSPORT_SOFTAP);
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        
        wifi_prov_mgr_deinit();

        /* Start Wi-Fi station */
        wifi_init_sta();
    }

    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(10000)); 

     //ret = init_spiffs();
    //ESP_ERROR_CHECK(ret);
            mqtt_app_start();



    /* Création de la tâche du serveur HTTP */
    xTaskCreate(webserver_task, "webserver_task", 8192, NULL, 6, NULL);



    /* Start main application now */
    while (1) {
       
        int button_state = gpio_get_level(reset_wifi);
        if (button_state == 0) {  // Check if the button is pressed
            ESP_LOGI("Main", " reset wifi...!");
            // procesuure de reprovisionning
        /* Resetting provisioning state machine to enable re-provisioning */
        wifi_prov_mgr_reset_sm_state_for_reprovision();
         /* Wait for Wi-Fi connection */
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
        }
        else {






    //wifi_connection();
   // mqtt_app_start();
   esp_get_free_heap_size();
    if (WIFI_EVENT_STA_CONNECTED ) 
    {




        ESP_LOGI(TAG, " Conneeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeect...\n");



        }








        }  
        vTaskDelay(pdMS_TO_TICKS(100)); 

        
    }



}
