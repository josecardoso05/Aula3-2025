#include "sjf.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @brief First-In-First-Out (FIFO) scheduling algorithm.
 *
 * This function implements the FIFO scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * If the CPU is idle, it selects the next task to run based on the order they were added
 * to the ready queue. The task that has been in the queue the longest is selected to run next.
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */


pcb_t *encontraMenorTempo(queue_t *rq){

    if (rq->head == NULL) {
        return NULL;
    }

    queue_elem_t *percorreLista = rq->head;
    queue_elem_t *tarefaMin = percorreLista;

    while (percorreLista!=NULL){
        if (percorreLista->pcb->time_ms < tarefaMin->pcb->time_ms ){ //vai comparar os tempos das tarefas para encontrar o menor tempo

            tarefaMin=percorreLista;
        }

        percorreLista= percorreLista->next;
    }

    queue_elem_t *tarefaParaRemov = remove_queue_elem(rq,tarefaMin); //remove o elemento de menor tempo da queue
    pcb_t *retornoTarefa = tarefaParaRemov->pcb;    //vai armazenar o pcb da tarefa removida

    free(tarefaParaRemov);

    return retornoTarefa;
}


void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) { //verifica se existe há algum processo a correr, (ponteiro guardado em *cpu_task não é NULL)
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                    .pid = (*cpu_task)->pid,
                    .request = PROCESS_REQUEST_DONE,
                    .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }
    if (*cpu_task == NULL) {            // If CPU is idle
        *cpu_task = encontraMenorTempo(rq);   // Get next task from ready queue (dequeue from head)
    }
}