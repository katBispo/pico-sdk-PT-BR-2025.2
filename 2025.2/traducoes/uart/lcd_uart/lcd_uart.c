/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Código de exemplo para controlar um painel LCD 16x2
   usando um “backpack” LCD TTL da Adafruit.

   Opcionalmente, o backpack pode ser conectado ao VBUS (pino 40) em 5V
   se o Pico em questão for alimentado por USB, para obter maior brilho.

   Se isso for feito, nenhuma outra conexão deve ser feita ao backpack
   além das listadas abaixo, pois os níveis lógicos do backpack irão mudar.

   Conexões na placa Raspberry Pi Pico (outras placas podem variar):

   GPIO 8 (pino 11) -> RX no backpack
   3.3V (pino 36)   -> 3.3V no backpack
   GND (pino 38)    -> GND no backpack
*/

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"

// deixa a uart0 livre para stdio
#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// comandos básicos
#define LCD_DISPLAY_ON 0x42
#define LCD_DISPLAY_OFF 0x46
#define LCD_SET_BRIGHTNESS 0x99
#define LCD_SET_CONTRAST 0x50
#define LCD_AUTOSCROLL_ON 0x51
#define LCD_AUTOSCROLL_OFF 0x52
#define LCD_CLEAR_SCREEN 0x58
#define LCD_SET_SPLASH 0x40

// comandos de cursor
#define LCD_SET_CURSOR_POS 0x47
#define LCD_CURSOR_HOME 0x48
#define LCD_CURSOR_BACK 0x4C
#define LCD_CURSOR_FORWARD 0x4D
#define LCD_UNDERLINE_CURSOR_ON 0x4A
#define LCD_UNDERLINE_CURSOR_OFF 0x4B
#define LCD_BLOCK_CURSOR_ON 0x53
#define LCD_BLOCK_CURSOR_OFF 0x54

// comandos RGB
#define LCD_SET_BACKLIGHT_COLOR 0xD0
#define LCD_SET_DISPLAY_SIZE 0xD1

// altere para 0 se o display não suportar RGB
#define LCD_IS_RGB 1

void lcd_write(uint8_t cmd, uint8_t *buf, uint8_t buflen)
{
    // todos os comandos são prefixados com 0xFE
    const uint8_t pre = 0xFE;
    uart_write_blocking(UART_ID, &pre, 1);
    uart_write_blocking(UART_ID, &cmd, 1);
    uart_write_blocking(UART_ID, buf, buflen);
    sleep_ms(10); // dá um pequeno tempo para o display processar
}

void lcd_set_size(uint8_t w, uint8_t h)
{
    // define as dimensões do display
    uint8_t buf[] = {w, h};
    lcd_write(LCD_SET_DISPLAY_SIZE, buf, 2);
}

void lcd_set_contrast(uint8_t contrast)
{
    // define o contraste do display
    lcd_write(LCD_SET_CONTRAST, &contrast, 1);
}

void lcd_set_brightness(uint8_t brightness)
{
    // define o brilho do backlight
    lcd_write(LCD_SET_BRIGHTNESS, &brightness, 1);
}

void lcd_set_cursor(bool is_on)
{
    // defina is_on como true se quiser mostrar o cursor em bloco piscante e sublinhado
    if (is_on)
    {
        lcd_write(LCD_BLOCK_CURSOR_ON, NULL, 0);
        lcd_write(LCD_UNDERLINE_CURSOR_ON, NULL, 0);
    }
    else
    {
        lcd_write(LCD_BLOCK_CURSOR_OFF, NULL, 0);
        lcd_write(LCD_UNDERLINE_CURSOR_OFF, NULL, 0);
    }
}

void lcd_set_backlight(bool is_on)
{
    // liga (true) ou desliga (false) o backlight
    if (is_on)
    {
        lcd_write(LCD_DISPLAY_ON, (uint8_t *)0, 1);
    }
    else
    {
        lcd_write(LCD_DISPLAY_OFF, NULL, 0);
    }
}

void lcd_clear()
{
    // limpa o conteúdo do display
    lcd_write(LCD_CLEAR_SCREEN, NULL, 0);
}

void lcd_cursor_reset()
{
    // reseta o cursor para a posição (1, 1)
    lcd_write(LCD_CURSOR_HOME, NULL, 0);
}

#if LCD_IS_RGB
void lcd_set_backlight_color(uint8_t r, uint8_t g, uint8_t b)
{
    // suportado apenas em displays RGB!
    uint8_t buf[] = {r, g, b};
    lcd_write(LCD_SET_BACKLIGHT_COLOR, buf, 3);
}
#endif

void lcd_init()
{
    lcd_set_backlight(true);
    lcd_set_size(LCD_WIDTH, LCD_HEIGHT);
    lcd_set_contrast(155);
    lcd_set_brightness(255);
    lcd_set_cursor(false);
}

int main()
{
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    uart_set_translate_crlf(UART_ID, false);
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));

    bi_decl(bi_1pin_with_func(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN)));

    lcd_init();

    // define a sequência de inicialização e salva na EEPROM
    // nem mais nem menos que 32 caracteres; se faltar, preencha o restante com espaços
    uint8_t splash_buf[] = "Hello LCD, from Pi Towers!      ";
    lcd_write(LCD_SET_SPLASH, splash_buf, LCD_WIDTH * LCD_HEIGHT);

    lcd_cursor_reset();
    lcd_clear();

#if LCD_IS_RGB
    uint8_t i = 0; // não tem problema se isso estourar e voltar, estamos usando seno
    const float frequency = 0.1f;
    uint8_t red, green, blue;
#endif

    while (1)
    {
        // envia quaisquer caracteres vindos do stdio diretamente para o backpack
        char c = getchar();
        // quaisquer bytes não precedidos por 0xFE (comando especial)
        // são interpretados como texto a ser exibido no backpack,
        // então apenas enviamos o caractere pela UART!
        if (c < 128)
            uart_putc_raw(UART_ID, c); // ignora caracteres extras não-ASCII
#if LCD_IS_RGB
        // muda a cor do display a cada tecla pressionada, estilo arco-íris!
        red = (uint8_t)(sin(frequency * i + 0) * 127 + 128);
        green = (uint8_t)(sin(frequency * i + 2) * 127 + 128);
        blue = (uint8_t)(sin(frequency * i + 4) * 127 + 128);
        lcd_set_backlight_color(red, green, blue);
        i++;
#endif
    }
}
