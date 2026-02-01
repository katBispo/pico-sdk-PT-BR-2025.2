/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Usa o recurso de “sniff” do mecanismo de DMA para calcular um CRC32
// sobre os dados em um buffer.
// Nota: isso NÃO faz uma cópia real dos dados; ele “transfere” todos os
// dados para um único byte de destino fictício, apenas para poder percorrer
// os dados de entrada usando o DMA.
// Se for necessária uma cópia de dados *com* um sniff de CRC32, o endereço
// inicial de um buffer de destino com tamanho adequado deve ser fornecido
// e o 'write_increment' deve ser configurado como true (veja abaixo).

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"

#define CRC32_INIT ((uint32_t)-1l)

#define DATA_TO_CHECK_LEN 9
#define CRC32_LEN 4
#define TOTAL_LEN (DATA_TO_CHECK_LEN + CRC32_LEN)

// Dados de teste de CRC comumente usados e também espaço para o valor do CRC
static uint8_t src[TOTAL_LEN] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00};
static uint8_t dummy_dst[1];

// Isso usa um polinômio padrão com a direção de deslocamento alternativa
// “reversa”.
// É possível usar um algoritmo não reverso aqui, mas a configuração do
// sniff do DMA abaixo precisaria ser modificada para permanecer consistente
// e permitir que a verificação passe.
static uint32_t soft_crc32_block(uint32_t crc, uint8_t *bytp, uint32_t length)
{
    while (length--)
    {
        uint32_t byte32 = (uint32_t)*bytp++;

        for (uint8_t bit = 8; bit; bit--, byte32 >>= 1)
        {
            crc = (crc >> 1) ^ (((crc ^ byte32) & 1ul) ? 0xEDB88320ul : 0ul);
        }
    }
    return crc;
}

int main()
{
    uint32_t crc_res;

    stdio_init_all();

    // Calcula e anexa o CRC
    crc_res = soft_crc32_block(CRC32_INIT, src, DATA_TO_CHECK_LEN);
    *((uint32_t *)&src[DATA_TO_CHECK_LEN]) = crc_res;

    printf("Buffer para DMA: ");
    for (int i = 0; i < TOTAL_LEN; i++)
    {
        printf("0x%02x ", src[i]);
    }
    printf("\n");

    // Descomente a próxima linha para corromper o buffer propositalmente
    // src[0]++;  // modifique qualquer byte, de qualquer forma, para quebrar a verificação CRC32

    // Obtém um canal livre; chama panic() se não houver nenhum disponível
    int chan = dma_claim_unused_channel(true);

    // Transferências de 8 bits. O endereço de leitura é incrementado após
    // cada transferência, mas o endereço de escrita permanece inalterado,
    // apontando para o destino fictício.
    // Nenhum DREQ é selecionado, então o DMA transfere o mais rápido possível.
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    // Configuração específica do sniff de CRC32 (com reversão de bits)
    channel_config_set_sniff_enable(&c, true);
    dma_sniffer_set_data_accumulator(CRC32_INIT);
    dma_sniffer_set_output_reverse_enabled(true);
    dma_sniffer_enable(chan, DMA_SNIFF_CTRL_CALC_VALUE_CRC32R, true);

    dma_channel_configure(
        chan,      // Canal a ser configurado
        &c,        // A configuração que acabamos de criar
        dummy_dst, // Endereço de escrita (fixo)
        src,       // Endereço inicial de leitura
        TOTAL_LEN, // Número total de transferências incluindo o CRC anexado; cada uma é 1 byte
        true       // Iniciar imediatamente
    );

    // Poderíamos fazer outra coisa enquanto o DMA executa a transferência.
    // Neste caso, o processador não tem mais nada para fazer, então apenas
    // aguardamos o DMA terminar.
    dma_channel_wait_for_finish_blocking(chan);

    uint32_t sniffed_crc = dma_sniffer_get_data_accumulator();
    printf(
        "DMA sniff concluído para buffer de %d bytes, valor do acumulador do DMA sniff: 0x%x\n",
        TOTAL_LEN,
        sniffed_crc);

    if (0ul == sniffed_crc)
    {
        printf("Verificação CRC32 está correta\n");
    }
    else
    {
        printf("ERRO - Verificação CRC32 FALHOU!\n");
    }
}
