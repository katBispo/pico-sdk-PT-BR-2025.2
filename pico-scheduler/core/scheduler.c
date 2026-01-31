#include "scheduler.h"
#include "pico/time.h"
#include "console.h"

#define MAX_TAREFAS 4

/**
 * Estrutura que representa uma tarefa periodica
 */
typedef struct {
    funcao_tarefa_t tarefa;
    uint32_t intervalo_ms;
    repeating_timer_t temporizador;
    bool ativa;
} tarefa_periodica_t;

static tarefa_periodica_t tarefas[MAX_TAREFAS];
static uint8_t total_tarefas = 0;

/**
 * Callback executado automaticamente pelo timer do Pico SDK
 */
static bool callback_tarefa(repeating_timer_t *rt) {
    tarefa_periodica_t *tarefa_atual = (tarefa_periodica_t *) rt->user_data;

    if (tarefa_atual && tarefa_atual->tarefa) {
        tarefa_atual->tarefa();
    }

    return true;
}

void scheduler_init(void) {
    console_log("Escalonador inicializado");
    total_tarefas = 0;
}

void scheduler_add_task(funcao_tarefa_t tarefa, uint32_t intervalo_ms) {
    if (total_tarefas >= MAX_TAREFAS) {
        console_log("Erro: limite maximo de tarefas atingido");
        return;
    }

    tarefas[total_tarefas].tarefa = tarefa;
    tarefas[total_tarefas].intervalo_ms = intervalo_ms;
    tarefas[total_tarefas].ativa = true;

    add_repeating_timer_ms(
        intervalo_ms,
        callback_tarefa,
        &tarefas[total_tarefas],
        &tarefas[total_tarefas].temporizador
    );

    total_tarefas++;
}

void scheduler_start(void) {
    console_log("Escalonador em execucao");

    while (true) {
        tight_loop_contents();
    }
}