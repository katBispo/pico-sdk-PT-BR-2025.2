/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"

// Dispositivos Pico W usam um GPIO no chip de Wi-Fi para o LED;
// então, ao compilar para Pico W, CYW43_WL_GPIO_LED_PIN será definido
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Realiza a inicialização
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Um dispositivo como o Pico, que usa um GPIO para o LED, definirá PICO_DEFAULT_LED_PIN,
    // então podemos usar as funções normais de GPIO para ligar e desligar o LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Para dispositivos Pico W, precisamos inicializar o driver etc.
    return cyw43_arch_init();
#endif
}

// Liga ou desliga o LED
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Apenas define o GPIO como ligado ou desligado
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Solicita ao “driver” do Wi-Fi que defina o GPIO como ligado ou desligado
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

int main() {
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
    while (true) {
        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);
        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);
    }
}
