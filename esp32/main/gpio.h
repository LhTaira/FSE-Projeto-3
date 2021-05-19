#ifndef GPIO_H
#define GPIO_H

#define LED_1 2
#define BOTAO 0


void gpio_isr_handler(void *args);

void trataInterrupcaoBotao(void *params);
void toggle_led();
void gpio_init();

#endif


