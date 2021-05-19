#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/semphr.h"
#include "http_client.h"
#include "mqtt.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi.h"

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;
xSemaphoreHandle registerDeviceSemaphore;

void conectadoWifi(void* params) {
  while (true) {
    if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY)) {
      mqtt_start();
    }
  }
}

void trataComunicacaoComServidor(void* params) {
  // char mensagem[50];
  xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY);

  mqtt_register_device();

  xSemaphoreTake(registerDeviceSemaphore, portMAX_DELAY);
  while (true) {
    int temperature = 20 + (int)((float)rand() / (float)(RAND_MAX / 10.0));
    int humidity = 20 + (int)((float)rand() / (float)(RAND_MAX / 10.0));

    mqtt_send_temperature(temperature);
    mqtt_send_humidity(humidity);
    mqtt_send_state(1);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  nvs_init();

  conexaoWifiSemaphore = xSemaphoreCreateBinary();
  conexaoMQTTSemaphore = xSemaphoreCreateBinary();
  registerDeviceSemaphore = xSemaphoreCreateBinary();
  wifi_start();

  xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);
  xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096,
              NULL, 1, NULL);
}
