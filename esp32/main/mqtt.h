#ifndef MQTT_H
#define MQTT_H

void mqtt_start();

void mqtt_envia_mensagem(char * topico, char * mensagem);
void mqtt_send_temperature(int temperature);
void mqtt_send_humidity(int humidity);
void mqtt_send_state(int state);
void mqtt_register_device();

#endif
