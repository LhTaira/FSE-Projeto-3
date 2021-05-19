#include "gpio.h"

#include <stdio.h>
#include <string.h>
#include "mqtt.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

xQueueHandle filaDeInterrupcao;

void gpio_isr_handler(void *args) {
  int pino = (int)args;
  xQueueSendFromISR(filaDeInterrupcao, &pino, NULL);
}

void trataInterrupcaoBotao(void *params) {
  int pino;
  int contador = 0;

  while (true) {
    if (xQueueReceive(filaDeInterrupcao, &pino, portMAX_DELAY)) {
      // De-bouncing
      int estado = gpio_get_level(pino);
      if (estado == 1) {
        gpio_isr_handler_remove(pino);
        while (gpio_get_level(pino) == estado) {
          vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        contador++;
        mqtt_send_state(estado);
        printf("Os botões foram acionados %d vezes. Botão: %d\n", contador,
               pino);

        // Habilitar novamente a interrupção
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_isr_handler_add(pino, gpio_isr_handler, (void *)pino);
      }
    }
  }
}

void toggle_led() {}
void gpio_init() {
  // Configuração dos pinos dos LEDs
  gpio_pad_select_gpio(LED_1);
  // Configura os pinos dos LEDs como Output
  gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);

  // Configuração do pino do Botão
  gpio_pad_select_gpio(BOTAO);
  // Configura o pino do Botão como Entrada
  gpio_set_direction(BOTAO, GPIO_MODE_INPUT);
  // Configura o resistor de Pulldown para o botão (por padrão a entrada estará
  // em Zero)
  gpio_pulldown_en(BOTAO);
  // Desabilita o resistor de Pull-up por segurança.
  gpio_pullup_dis(BOTAO);

  // Configura pino para interrupção
  gpio_set_intr_type(BOTAO, GPIO_INTR_POSEDGE);

  filaDeInterrupcao = xQueueCreate(10, sizeof(int));
  xTaskCreate(trataInterrupcaoBotao, "TrataBotao", 2048, NULL, 1, NULL);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(BOTAO, gpio_isr_handler, (void *)BOTAO);
}
