#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"

#include ROOM_LEN 45

void nvs_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void nvs_save_room(char *room) {
    nvs_handle partition_handle;
    esp_err_t res_nvs = nvs_open("storage", NVS_READWRITE, &partition_handle);

    if(res_nvs == ESP_ERR_NVS_NOT_FOUND)
        ESP_LOGE("NVS", "Namespace: storage, not found");

    esp_err_t res = nvs_set_str(partition_handle, "room", room);

    if(res != ESP_OK)
        ESP_LOGE("NVS", "Não foi possível escrever no NVS");

    nvs_commit(partition_handle);
    nvs_close(partition_handle);
}

char* nvs_get_room() {
    char *value = malloc(ROOM_LEN);
    nvs_handle partition_handle;

    esp_err_t res_nvs = nvs_open("storage", NVS_READWRITE, &partition_handle);

    if(res_nvs == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Namespace: storage, not found");
        return NULL:
    }
    esp_err_t res = nvs_get_str(partition_handl, "room", &value);

    switch (res) {
        case ESP_OK:
            return value;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE("NVS", "Valor não encontrado");
            free(value);
            return NULL;
        default:
            ESP_LOGE("NVS", "Erro ao acessar o NVS (%s)", esp_err_to_name(res));
            free(value);
            return NULL;
    }
}