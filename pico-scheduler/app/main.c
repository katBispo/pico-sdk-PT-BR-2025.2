#include "scheduler.h"
#include "console.h"

/**
 * Tarefa executada a cada 1 segundo
 */
void tarefa_um(void) {
    console_log("Tarefa 1 executando a cada 1 segundo");
}

/**
 * Tarefa executada a cada 2 segundos
 */
void tarefa_dois(void) {
    console_log("Tarefa 2 executando a cada 2 segundos");
}

int main() {
    console_init();
    scheduler_init();

    scheduler_add_task(tarefa_um, 1000);
    scheduler_add_task(tarefa_dois, 2000);

    scheduler_start();
    return 0;
}