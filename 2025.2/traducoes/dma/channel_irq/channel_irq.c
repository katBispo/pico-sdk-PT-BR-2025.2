/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Mostra como reconfigurar e reiniciar um canal dentro do manipulador
// de interrupção de conclusão do canal.
//
// Nosso canal DMA irá transferir dados para uma máquina de estado do PIO,
// que está configurada para serializar os bits brutos que enviamos, um por um.
// Vamos usar isso para fazer um PWM simples em um LED, enviando repetidamente
// valores com o equilíbrio correto entre bits 1 e 0.
// (nota: existem formas melhores de fazer PWM com PIO
// -- veja o exemplo de PWM com PIO).
//
// Quando o canal tiver enviado uma quantidade predeterminada de dados,
// ele irá parar e levantar uma flag de interrupção.
// O processador entrará no manipulador de interrupção em resposta a isso,
// onde o canal será reconfigurado e reiniciado.
// Esse processo se repete.

#include <stdio.h>
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pio_serialiser.pio.h"

// O PIO envia um bit a cada 10 ciclos do clock do sistema.
// O DMA envia o mesmo valor de 32 bits 10.000 vezes antes de parar.
// Isso significa que percorremos os 32 níveis de PWM aproximadamente
// uma vez por segundo.
#define PIO_SERIAL_CLKDIV 10.f
#define PWM_REPEAT_COUNT 10000
#define N_PWM_LEVELS 32

int dma_chan;

void dma_handler()
{
    static int pwm_level = 0;
    static uint32_t wavetable[N_PWM_LEVELS];
    static bool first_run = true;
    // A entrada número `i` possui `i` bits 1 e `(32 - i)` bits 0.
    if (first_run)
    {
        first_run = false;
        for (int i = 0; i < N_PWM_LEVELS; ++i)
            wavetable[i] = ~(~0u << i);
    }

    // Limpa a requisição de interrupção.
    dma_hw->ints0 = 1u << dma_chan;
    // Fornece ao canal uma nova entrada da wavetable para leitura
    // e o reaciona
    dma_channel_set_read_addr(dma_chan, &wavetable[pwm_level], true);

    pwm_level = (pwm_level + 1) % N_PWM_LEVELS;
}

int main()
{
#ifndef PICO_DEFAULT_LED_PIN
#warning O exemplo dma/channel_irq requer uma placa com um LED padrão
#else
    // Configura uma máquina de estado do PIO para serializar nossos bits
    uint offset = pio_add_program(pio0, &pio_serialiser_program);
    pio_serialiser_program_init(pio0, 0, offset, PICO_DEFAULT_LED_PIN, PIO_SERIAL_CLKDIV);

    // Configura um canal para escrever repetidamente a mesma palavra (32 bits)
    // no FIFO TX da SM0 do PIO0, sincronizado pelo sinal de requisição
    // de dados desse periférico.
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_PIO0_TX0);

    dma_channel_configure(
        dma_chan,
        &c,
        &pio0_hw->txf[0], // Endereço de escrita (só precisa ser definido uma vez)
        NULL,             // Ainda não fornece um endereço de leitura
        PWM_REPEAT_COUNT, // Escreve o mesmo valor várias vezes, depois para e gera interrupção
        false             // Não iniciar ainda
    );

    // Diz ao DMA para gerar a IRQ 0 quando o canal terminar um bloco
    dma_channel_set_irq0_enabled(dma_chan, true);

    // Configura o processador para executar dma_handler()
    // quando a IRQ 0 do DMA for acionada
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Chama manualmente o handler uma vez para iniciar a primeira transferência
    dma_handler();

    // A partir daqui, tudo é controlado por interrupções.
    // O processador tem tempo para sentar e pensar na aposentadoria antecipada —
    // talvez abrir uma padaria?
    while (true)
        tight_loop_contents();
#endif
}
