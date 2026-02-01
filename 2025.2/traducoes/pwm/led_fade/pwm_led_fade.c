/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Fade an LED between low and high brightness. An interrupt handler updates
// the PWM slice's output level each time the counter wraps.

#include "pico/stdlib.h"
#include <stdio.h>
#include "pico/time.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

#ifdef PICO_DEFAULT_LED_PIN
void on_pwm_wrap() {
    static int fade = 0;
    static bool going_up = true;
    // Limpa a flag de interrupção que nos trouxe até aqui
    pwm_clear_irq(pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN));

    if (going_up) {
        ++fade;
        if (fade > 255) {
            fade = 255;
            going_up = false;
        }
    } else {
        --fade;
        if (fade < 0) {
            fade = 0;
            going_up = true;
        }
    }
    // Eleva ao quadrado o valor de fade para que o brilho do LED pareça mais linear
    // Observe que esse intervalo corresponde ao valor de wrap
    pwm_set_gpio_level(PICO_DEFAULT_LED_PIN, fade * fade);
}
#endif

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning pwm/led_fade example requires a board with a regular LED
#else
    // Informa ao pino do LED que o PWM é o responsável pelo seu valor.
    gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);
    // Descobre qual slice acabamos de conectar ao pino do LED
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

    // Faz o mascaramento da saída de IRQ do nosso slice na única linha de interrupção do bloco PWM
    // e registra nosso tratador de interrupção
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), on_pwm_wrap);
    irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);

    // Obtém alguns valores padrão razoáveis para a configuração do slice. Por padrão, o
    // contador pode fazer wrap em todo o seu intervalo máximo (0 a 2**16-1)
    pwm_config config = pwm_get_default_config();
    // Define o divisor: reduz o clock do contador para sysclock/este valor
    pwm_config_set_clkdiv(&config, 4.f);
    // Carrega a configuração no nosso slice de PWM e inicia sua execução.
    pwm_init(slice_num, &config, true);

    // A partir deste ponto, tudo acontece no tratador de interrupção do PWM, então
    // podemos apenas aguardar
    while (1)
        tight_loop_contents();
#endif
}
