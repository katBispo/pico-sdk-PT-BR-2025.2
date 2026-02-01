/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

/// \tag::uart_advanced[]

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// Estamos usando os pinos 0 e 1, mas veja a tabela de seleção de função dos GPIOs
// no datasheet para informações sobre quais outros pinos podem ser usados.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;

// Manipulador de interrupção de RX
void on_uart_rx()
{
    while (uart_is_readable(UART_ID))
    {
        uint8_t ch = uart_getc(UART_ID);
        // Podemos enviá-lo de volta?
        if (uart_is_writable(UART_ID))
        {
            // Altere-o um pouco primeiro!
            ch++;
            uart_putc(UART_ID, ch);
        }
        chars_rxed++;
    }
}

int main()
{
    // Configura nossa UART com uma taxa de baud básica.
    uart_init(UART_ID, 2400);

    // Configura os pinos TX e RX usando a seleção de função no GPIO
    // Veja o datasheet para mais informações sobre seleção de função
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    // Na verdade, queremos uma velocidade diferente
    // A chamada retornará a taxa de baud real selecionada, que será o mais próxima
    // possível da solicitada
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Configura o controle de fluxo CTS/RTS da UART, não queremos isso, então desative
    uart_set_hw_flow(UART_ID, false, false);

    // Configura o formato dos dados
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Desativa os FIFOs - queremos fazer isso caractere por caractere
    uart_set_fifo_enabled(UART_ID, false);

    // Configura uma interrupção de RX
    // Precisamos configurar o manipulador primeiro
    // Seleciona a interrupção correta para a UART que estamos usando
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // E configura e habilita os manipuladores de interrupção
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Agora habilita a UART para enviar interrupções - apenas RX
    uart_set_irq_enables(UART_ID, true, false);

    // OK, tudo configurado.
    // Vamos enviar uma string básica e então rodar um loop esperando por interrupções de RX
    // O manipulador irá contá-las, mas também refletirá os dados recebidos de volta com uma leve alteração!
    uart_puts(UART_ID, "\nHello, uart interrupts\n");

    while (1)
        tight_loop_contents();
}

/// \end:uart_advanced[]
