
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>
#include "qrcode.h"

static const char *TAG = "app";

 char sec2_salt[16];
 char sec2_verifier[384];


 char EXAMPLE_PROV_SEC2_USERNAME[5];
 char EXAMPLE_PROV_SEC2_PWD[5];
char sec2_saltp[100];
char sec2_verifierp[30000];

// Fonction pour initialiser le NVS, lire les valeurs et les affecter aux tableaux correspondants
 void init_et_lire_nvs() {
    nvs_handle_t nvs_handle;
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Erase NVS partition and retry initialization
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();

    }
    if (ret != ESP_OK) {
        printf("Failed to initialize NVS\n");
        return;
    }

    // Open NVS namespace
      ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        
    }

    // Read values from NVS
    size_t taille_username = 32;
    size_t taille_password = 32;
    size_t taille_salt = 120;
    size_t taille_verifier = 3500;
    // Check for errors during data retrieval from NVS
    if (nvs_get_blob(nvs_handle, "username", EXAMPLE_PROV_SEC2_USERNAME, &taille_username) != ESP_OK ||
        nvs_get_blob(nvs_handle, "password", EXAMPLE_PROV_SEC2_PWD, &taille_password) != ESP_OK ||
        nvs_get_blob(nvs_handle, "salt", sec2_saltp, &taille_salt) != ESP_OK ||
        nvs_get_blob(nvs_handle, "verifier", sec2_verifierp, &taille_verifier) != ESP_OK) {
        printf("Error reading data from NVS\n");
        // Close NVS handle
        
        nvs_close(nvs_handle);
        return;
    }
    //printf("%s",sec2_verifierp);  
    //printf("%s",sec2_saltp);
   
   char *token;
    int i = 0;

    // Copie de la chaîne originale pour éviter de la modifier
    char *salt_copy = strdup(sec2_saltp);

    // Séparation de la chaîne en tokens basés sur la virgule et l'espace
    token = strtok(salt_copy, ", ");
    while (token != NULL && i < 16) {
        // Conversion du token en nombre hexadécimal
        sec2_salt[i++] = (unsigned char)strtol(token, NULL, 0);
        token = strtok(NULL, ", ");
    }

    // Libération de la mémoire allouée pour la copie de la chaîne
    free(salt_copy);

    // Affichage du tableau hexadécimal
   // printf("Tableau hexadécimal : ");
    //for (i = 0; i < 16; i++) {
      //  printf("0x%02x ", sec2_salt[i]);
   // }
    //printf("\n");
//////////////////////////////////////////////////////

char *tokenv;
    int iv = 0;

    // Copie de la chaîne originale pour éviter de la modifier
    char *verif_copy = strdup(sec2_verifierp);

    // Séparation de la chaîne en tokens basés sur la virgule et l'espace
    tokenv = strtok(verif_copy, ",");
    while (tokenv != NULL && iv < 384) {
        // Conversion du token en nombre hexadécimal
        sec2_verifier[iv++] =(unsigned char)strtol(tokenv, NULL, 0);
        tokenv = strtok(NULL, ",");
    }

    // Libération de la mémoire allouée pour la copie de la chaîne
    free(verif_copy);

    // Affichage du tableau hexadécimal
    //printf("Tableau hexadécimal : ");
    //for (iv = 0; iv < 384; iv++) {
      //  printf("0x%02x ", sec2_verifier[iv]);
   // }
    //printf("\n");
 



} 

 
 

  








/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static esp_err_t example_get_sec2_salt(   char **salt, uint16_t *salt_len) {
    ESP_LOGI(TAG, "Mode de développement : utilisation d'un sel codé en dur");
    *salt = sec2_salt;
    *salt_len = sizeof(sec2_salt); // Obtenez la longueur du sel
   // printf("Longueur du sel : %d\n", *salt_len);
    //printf("sec2_salt : %s\n", sec2_salt);


    return ESP_OK;
}

static esp_err_t example_get_sec2_verifier( char **verifier, uint16_t *verifier_len) {
    ESP_LOGI(TAG, "Mode de développement : utilisation d'un vérificateur codé en dur");
    *verifier = sec2_verifier;
    *verifier_len = sizeof(sec2_verifier); // Obtenez la longueur du vérificateur
   // printf("Longueur du verif : %d\n", *verifier_len);
    //printf("sec2_salt : %s\n", sec2_verifier);
    return ESP_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//








/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

#define PROV_QR_VERSION         "v1"
#define PROV_TRANSPORT_SOFTAP   "softap"
  #define QRCODE_BASE_URL         "https://espressif.github.io/esp-jumpstart/qrcode.html"

/* Event handler for catching system events */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
    static int retries;
#endif
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
                retries++;
                if (retries >= CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT) {
                    ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                    wifi_prov_mgr_reset_sm_state_on_failure();
                    retries = 0;
                }
#endif
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
                retries = 0;
#endif
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Connected!");
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);

    } else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {
        switch (event_id) {
            case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
                ESP_LOGI(TAG, "Secured session established!");
                break;
            case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
                ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
                break;
            case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
                ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
                break;
            default:
                break;
        }
    }
}

static void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
    if (!name || !transport) {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop) {

        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                    ",\"username\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
                    PROV_QR_VERSION, name, username, pop, transport);
    } else {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                    ",\"transport\":\"%s\"}",
                    PROV_QR_VERSION, name, transport);
    }
#ifdef CONFIG_EXAMPLE_PROV_SHOW_QR
    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    esp_qrcode_generate(&cfg, payload);
#endif /* CONFIG_APP_WIFI_PROV_SHOW_QR */
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}