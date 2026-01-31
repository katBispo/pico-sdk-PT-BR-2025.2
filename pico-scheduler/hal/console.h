#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * Inicializa o console (stdout)
 */
void console_init(void);

/**
 * Exibe uma mensagem no console
 * @param mensagem Texto a ser exibido
 */
void console_log(const char *mensagem);

#endif