/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Gera sinais PWM nos pinos 0 e 1

#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    /// \tag::setup_pwm[]

    // Informa aos GPIOs 0 e 1 que eles estão alocados para o PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    // Descobre qual “slice” de PWM está conectado ao GPIO 0 (é o slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);

    // Define um período de 4 ciclos (de 0 a 3, inclusive)
    pwm_set_wrap(slice_num, 3);
    // Define a saída do canal A em nível alto por 1 ciclo antes de cair
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // Define a saída inicial do canal B em nível alto por 3 ciclos antes de cair
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
    // Inicia a execução do PWM
    pwm_set_enabled(slice_num, true);
    /// \end::setup_pwm[]

    // Observe que também podemos usar pwm_set_gpio_level(gpio, x), que identifica
    // o slice e o canal corretos para um determinado GPI
}
