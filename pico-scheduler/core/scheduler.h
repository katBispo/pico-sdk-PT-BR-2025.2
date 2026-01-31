#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

/**
 * Tipo que representa uma funcao de tarefa
 */
typedef void (*funcao_tarefa_t)(void);

/**
 * Inicializa o escalonador
 */
void scheduler_init(void);

/**
 * Adiciona uma tarefa periodica ao escalonador
 * @param tarefa Funcao a ser executada
 * @param intervalo_ms Intervalo em milissegundos
 */
void scheduler_add_task(funcao_tarefa_t tarefa, uint32_t intervalo_ms);

/**
 * Inicia o escalonador
 */
void scheduler_start(void);

#endif