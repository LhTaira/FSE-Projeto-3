#include "mqtt.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "nvs.h"
#include "gpio.h"

#define TAG "MQTT"

#define MATRICULA "170109208"

#define TOPIC_REGISTER 0
#define TOPIC_TEMPERATURE 1
#define TOPIC_HUMIDITY 2
#define TOPIC_STATE 3
#define TOPIC_DEVICE 4
#define TOPIC_LED 5

#define TOPIC_LEN 85

extern xSemaphoreHandle conexaoMQTTSemaphore;
extern xSemaphoreHandle registerDeviceSemaphore;
char *room;
esp_mqtt_client_handle_t client;

char *get_mac_address() {
  u_int8_t mac_address[6];
  char *formatted_address = malloc(16);

  esp_efuse_mac_get_default(mac_address);
  sprintf(formatted_address, "%x%x%x%x%x%x", mac_address[0], mac_address[1],
          mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

  return formatted_address;
}

char *get_topic(int topic_id) {
  char *MAC_ADDRESS = get_mac_address();
  char *topic = malloc(TOPIC_LEN);

  switch (topic_id) {
    case TOPIC_REGISTER:
      sprintf(topic, "fse2020/%s/dispositivos/%s", MATRICULA, MAC_ADDRESS);
      break;
    case TOPIC_TEMPERATURE:
      sprintf(topic, "fse2020/%s/%s/temperatura", MATRICULA, room);
      break;
    case TOPIC_HUMIDITY:
      sprintf(topic, "fse2020/%s/%s/umidade", MATRICULA, room);
      break;
    case TOPIC_STATE:
      sprintf(topic, "fse2020/%s/%s/estado", MATRICULA, room);
      break;
    default:
      sprintf(topic, "fse2020/%s/%s/led", MATRICULA, room);
  }

  return topic;
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      xSemaphoreGive(conexaoMQTTSemaphore);
      printf("Am i gay?\n");
      printf("Am i gay?2\n");
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      char *register_topic = get_topic(TOPIC_REGISTER);
      char *event_topic = (char *)malloc(event->topic_len*sizeof(char)+1);
      memcpy(event_topic, event->topic, event->topic_len);
      event_topic[event->topic_len*sizeof(char)] = '\0';
      
      if (strcmp(event_topic, register_topic) == 0) {

        cJSON *data_json = cJSON_Parse(event->data);
        room = (char*) malloc(strlen(cJSON_GetObjectItem(data_json, "room")->valuestring));
        strcpy(room, cJSON_GetObjectItem(data_json, "room")->valuestring);
        nvs_save_room(room);
        xSemaphoreGive(registerDeviceSemaphore);

        char *topic = get_topic(TOPIC_LED);
        esp_mqtt_client_subscribe(client, topic, 0);

      } else {
        toggle_led();
        printf("LED to be implemented.");
      }

      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
      break;
    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
  return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base,
           event_id);
  mqtt_event_handler_cb(event_data);
}

void mqtt_start() {
  esp_mqtt_client_config_t mqtt_config = {
      .uri = "mqtt://test.mosquitto.org",
  };
  client = esp_mqtt_client_init(&mqtt_config);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 client);
  esp_mqtt_client_start(client);
}

void mqtt_envia_mensagem(char *topico, char *mensagem) {
  int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, 1, 0);
  ESP_LOGI(TAG, "Mensagem enviada, ID: %d", message_id);
}

void mqtt_register_device() {
  if ((room = nvs_get_room()) != NULL)
    xSemaphoreGive(registerDeviceSemaphore);

  else {
    char *id = get_mac_address();
    char *topic = get_topic(TOPIC_REGISTER);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "id", id);

    char *json_data = cJSON_Print(data);

    esp_mqtt_client_publish(client, topic, json_data, strlen(json_data), 1, 0);
    printf("ASdasdasdasd %s\n", topic);
    esp_mqtt_client_subscribe(client, topic, 0);

    // Maybe let all topics be static so we don't need to keep freeing them in
    // multiple contexts
    free(id);
    free(topic);
    free(json_data);
  }
}

void mqtt_send_temperature(int temperature) {
  char *topic = get_topic(TOPIC_TEMPERATURE);
  cJSON *temperature_json = cJSON_CreateObject();
  cJSON_AddNumberToObject(temperature_json, "data", (double)temperature);

  char *data = cJSON_Print(temperature_json);

  esp_mqtt_client_publish(client, topic, data, strlen(data), 1, 0);

  free(topic);
  free(temperature_json);
  free(data);
}

void mqtt_send_humidity(int umidity) {
  char *topic = get_topic(TOPIC_HUMIDITY);
  cJSON *umidity_json = cJSON_CreateObject();
  cJSON_AddNumberToObject(umidity_json, "data", (double)umidity);

  char *data = cJSON_Print(umidity_json);

  esp_mqtt_client_publish(client, topic, data, strlen(data), 1, 0);

  free(topic);
  free(umidity_json);
  free(data);
}

void mqtt_send_state(int state) {
  char *topic = get_topic(TOPIC_STATE);
  cJSON *state_json = cJSON_CreateObject();
  cJSON_AddNumberToObject(state_json, "data", (double)state);

  char *data = cJSON_Print(state_json);

  esp_mqtt_client_publish(client, topic, data, strlen(data), 1, 0);

  free(topic);
  free(state_json);
  free(data);
}