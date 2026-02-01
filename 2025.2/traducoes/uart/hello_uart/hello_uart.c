/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

/// \tag::hello_uart[]

#define UART_ID uart0
#define BAUD_RATE 115200

// Estamos usando os pinos 0 e 1, mas veja a tabela de seleção de função dos GPIOs
// no datasheet para informações sobre quais outros pinos podem ser usados.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main() {
    // Configura a UART com a velocidade necessária.
    uart_init(UART_ID, BAUD_RATE);

    // Define os pinos TX e RX usando a seleção de função do GPIO
    // Consulte o datasheet para mais informações sobre a seleção de função
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    // Usa algumas das várias funções da UART para enviar dados
    // Em um sistema padrão, o printf também irá enviar dados pela UART padrão

    // Envia um caractere sem nenhuma conversão
    uart_putc_raw(UART_ID, 'A');

    // Envia um caractere realizando conversões CR/LF
    uart_putc(UART_ID, 'B');

    // Envia uma string, com conversões CR/LF
    uart_puts(UART_ID, " Hello, UART!\n");
    return 0;
}

/// \end::hello_uart[]
