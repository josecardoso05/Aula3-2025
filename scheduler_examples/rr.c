#include "rr.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"

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

pcb_t *timeSlice(queue_t *rq, pcb_t *cpu_task, uint32_t current_time_ms) {

    if (cpu_task) {
        if ((current_time_ms - cpu_task->slice_start_ms) >= 500) { // Verifica se o tempo do tarefa passa de 500ms
            cpu_task->slice_start_ms = current_time_ms;
            enqueue_pcb(rq, cpu_task);
            cpu_task = NULL;

        }
    }

    if (cpu_task == NULL) {
        cpu_task = dequeue_pcb(rq);

        if (cpu_task) {
            cpu_task->slice_start_ms = current_time_ms;
        }
    }

    return cpu_task;
}


void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) { //verifica se existe algum processo a correr <=> ponteiro guardado em *cpu_task não é NULL
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
            // Task finished
            // Send msg to application
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                    .pid = (*cpu_task)->pid,
                    .request = PROCESS_REQUEST_DONE,
                    .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {

                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            free(*cpu_task);
            *cpu_task = NULL;
        }
    }
    *cpu_task = timeSlice(rq, *cpu_task, current_time_ms);
}
