/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Usa dois canais de DMA para criar uma sequência programada de transferências
// de dados para a UART (uma operação de agregação de dados – data gather).
// Um canal é responsável por transferir os dados de fato, enquanto o outro
// reprograma repetidamente esse canal.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/structs/uart.h"

// Esses buffers serão enviados via DMA para a UART, um após o outro.

const char word0[] = "Transferring ";
const char word1[] = "one ";
const char word2[] = "word ";
const char word3[] = "at ";
const char word4[] = "a ";
const char word5[] = "time.\n";

// Observe a ordem dos campos aqui: é importante que o comprimento venha antes
// do endereço de leitura, porque o canal de controle irá escrever nos dois
// últimos registradores do alias 3 no canal de dados:
//           +0x0        +0x4          +0x8          +0xC (Trigger)
// Alias 0:  READ_ADDR   WRITE_ADDR    TRANS_COUNT   CTRL
// Alias 1:  CTRL        READ_ADDR     WRITE_ADDR    TRANS_COUNT
// Alias 2:  CTRL        TRANS_COUNT   READ_ADDR     WRITE_ADDR
// Alias 3:  CTRL        WRITE_ADDR    TRANS_COUNT   READ_ADDR
//
// Isso irá programar a contagem de transferências e o endereço de leitura do
// canal de dados, e acioná-lo. Quando o canal de dados terminar, ele irá
// reiniciar o canal de controle (via CHAIN_TO) para carregar as próximas duas
// palavras em seus registradores de controle.

const struct
{
    uint32_t len;
    const char *data;
} control_blocks[] = {
    {count_of(word0) - 1, word0}, // Ignora o terminador nulo
    {count_of(word1) - 1, word1},
    {count_of(word2) - 1, word2},
    {count_of(word3) - 1, word3},
    {count_of(word4) - 1, word4},
    {count_of(word5) - 1, word5},
    {0, NULL} // Gatilho nulo para encerrar a cadeia.
};

int main()
{
#ifndef uart_default
#warning O exemplo dma/control_blocks requer uma UART
#else
    stdio_init_all();
    puts("Exemplo de bloco de controle de DMA:");

    // ctrl_chan carrega blocos de controle no data_chan, que os executa.
    int ctrl_chan = dma_claim_unused_channel(true);
    int data_chan = dma_claim_unused_channel(true);

    // O canal de controle transfere duas palavras para os registradores de
    // controle do canal de dados e então para. O endereço de escrita faz wrap
    // em um limite de duas palavras (oito bytes), de modo que o canal de
    // controle escreva sempre nos mesmos dois registradores quando for
    // acionado novamente.

    dma_channel_config c = dma_channel_get_default_config(ctrl_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    channel_config_set_ring(&c, true, 3); // Limite de 1 << 3 bytes no ponteiro de escrita

    dma_channel_configure(
        ctrl_chan,
        &c,
        &dma_hw->ch[data_chan].al3_transfer_count, // Endereço inicial de escrita
        &control_blocks[0],                        // Endereço inicial de leitura
        2,                                         // Para após cada bloco de controle
        false                                      // Não iniciar ainda
    );

    // O canal de dados é configurado para escrever no FIFO da UART
    // (sincronizado pelo sinal de requisição de dados TX da UART) e então
    // encadear para o canal de controle quando concluir.
    // O canal de controle programa um novo endereço de leitura e comprimento
    // dos dados, e reaciona o canal de dados.

    c = dma_channel_get_default_config(data_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, uart_get_dreq(uart_default, true));
    // Aciona o ctrl_chan quando o data_chan concluir
    channel_config_set_chain_to(&c, ctrl_chan);
    // Gera a flag de IRQ quando 0 é escrito em um registrador de gatilho
    // (fim da cadeia):
    channel_config_set_irq_quiet(&c, true);

    dma_channel_configure(
        data_chan,
        &c,
        &uart_get_hw(uart_default)->dr,
        NULL, // O endereço inicial de leitura e a contagem de
        0,    // transferências não são importantes;
        false // o canal de controle irá reprogramá-los a cada vez.
    );

    // Tudo está pronto. Diz ao canal de controle para carregar o primeiro
    // bloco de controle. A partir daqui, tudo é automático.
    dma_start_channel_mask(1u << ctrl_chan);

    // O canal de dados irá acionar sua flag de IRQ quando receber um gatilho
    // nulo, indicando o fim da lista de blocos de controle. Vamos apenas
    // aguardar a flag de IRQ em vez de configurar um manipulador de interrupção.
    while (!(dma_hw->intr & 1u << data_chan))
        tight_loop_contents();
    dma_hw->ints0 = 1u << data_chan;

    puts("DMA finalizado.");
#endif
}
