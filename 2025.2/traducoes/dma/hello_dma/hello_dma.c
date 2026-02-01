/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Usa o DMA para copiar dados entre dois buffers na memória.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"

// Os dados serão copiados de src para dst
const char src[] = "Hello, world! (from DMA)";
char dst[count_of(src)];

int main()
{
    stdio_init_all();

    // Obtém um canal livre; chama panic() se não houver nenhum disponível
    int chan = dma_claim_unused_channel(true);

    // Transferências de 8 bits. Tanto o endereço de leitura quanto o de escrita
    // são incrementados após cada transferência
    // (cada um apontando para uma posição em src ou dst, respectivamente).
    // Nenhum DREQ é selecionado, então o DMA transfere o mais rápido possível.

    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(
        chan,          // Canal a ser configurado
        &c,            // A configuração que acabamos de criar
        dst,           // Endereço inicial de escrita
        src,           // Endereço inicial de leitura
        count_of(src), // Número de transferências; neste caso, cada uma é 1 byte
        true           // Iniciar imediatamente
    );

    // Poderíamos escolher fazer outra coisa enquanto o DMA executa a
    // transferência. Neste caso, o processador não tem mais nada para fazer,
    // então apenas aguardamos o DMA terminar.
    dma_channel_wait_for_finish_blocking(chan);

    // O DMA agora copiou o texto do buffer de transmissão (src) para o buffer
    // de recepção (dst), então podemos imprimi-lo a partir daí.
    puts(dst);
}
