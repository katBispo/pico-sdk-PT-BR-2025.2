#include "console.h"
#include "pico/stdlib.h"
#include <stdio.h>

void console_init(void) {
    stdio_init_all();
}

void console_log(const char *mensagem) {
    printf("[CONSOLE] %s\n", mensagem);
}