/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// Este exemplo gera uma saída PWM com diferentes duty cycles e usa
// outro slice de PWM em modo de entrada para medir o duty cycle. Você precisará
// conectar esses dois pinos com um jumper:
const uint OUTPUT_PIN = 2;
const uint MEASURE_PIN = 5;

float measure_duty_cycle(uint gpio) {
    // Apenas os pinos PWM do canal B podem ser usados como entradas.
    assert(pwm_gpio_to_channel(gpio) == PWM_CHAN_B);
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    // Conta uma vez a cada 100 ciclos em que a entrada PWM B estiver em nível alto
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_HIGH);
    pwm_config_set_clkdiv(&cfg, 100);
    pwm_init(slice_num, &cfg, false);
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    pwm_set_enabled(slice_num, true);
    sleep_ms(10);
    pwm_set_enabled(slice_num, false);
    float counting_rate = clock_get_hz(clk_sys) / 100;
    float max_possible_count = counting_rate * 0.01;
    return pwm_get_counter(slice_num) / max_possible_count;
}

const float test_duty_cycles[] = {
        0.f,
        0.1f,
        0.5f,
        0.9f,
        1.f
};

int main() {
    stdio_init_all();
    printf("\nPWM duty cycle measurement example\n");

    // Configura o slice de PWM e o coloca em execução
    const uint count_top = 1000;
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, count_top);
    pwm_init(pwm_gpio_to_slice_num(OUTPUT_PIN), &cfg, true);

    // Observe que ainda não estamos mexendo no outro pino — pinos PWM são saídas por
    // padrão, mas passam a ser entradas quando o modo do divisor é alterado de
    // “free-running”. Não é recomendável conectar duas saídas diretamente!
    gpio_set_function(OUTPUT_PIN, GPIO_FUNC_PWM);

    // Para cada duty cycle de teste, gera a saída nesse nível
    // e lê o duty cycle real usando o outro pino. Os dois valores
    // devem ser bem próximos!
    for (uint i = 0; i < count_of(test_duty_cycles); ++i) {
        float output_duty_cycle = test_duty_cycles[i];
        pwm_set_gpio_level(OUTPUT_PIN, (uint16_t) (output_duty_cycle * (count_top + 1)));
        float measured_duty_cycle = measure_duty_cycle(MEASURE_PIN);
        printf("Output duty cycle = %.1f%%, measured input duty cycle = %.1f%%\n",
               output_duty_cycle * 100.f, measured_duty_cycle * 100.f);
    }
}