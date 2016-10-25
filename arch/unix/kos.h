/**
 * Kevin's RTOS
 */
#ifndef _KOS_H_
#define _KOS_H_

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <ucontext.h>

#include "kos_settings.h"

typedef enum {
	TASK_READY,		/* sleep until at least next_tick */
	TASK_SEMAPHORE,		/* wait on semaphore */
	TASK_QUEUE,		/* wait on queue */
	TASK_ZOMBIE,		/* killed task */
} KOS_TaskStatus;

#define TASK_NAME_MAXLEN		8

typedef struct KOS_Task {
    void *sp;

    char name[TASK_NAME_MAXLEN];
    uint8_t nb_tick_to_wait;
    KOS_TaskStatus status;
    struct KOS_Task *next;
    void *status_pointer;

    uint8_t *stack_bottom;
    uint8_t *stack_top;

    ucontext_t uctx;
} KOS_Task;

typedef void (*KOS_TaskFn)(void);

extern KOS_Task *kos_current_task;

/**
 * Initializes the KOS kernel
 */
void kos_init(void);

/**
 * Creates a new task
 * Note: Not safe
 */
void kos_new_task(KOS_TaskFn task, const char *name, uint16_t size);

/**
 * Puts KOS in ISR mode
 * Note: Not safe, assumes non-nested isrs
 */
void kos_isr_enter(void);

/**
 * Leaves ISR mode, possibly executing the dispatcher
 * Note: Not safe, assumes non-nested isrs
 */
void kos_isr_exit(void);

#ifdef KOS_SEMAPHORE

typedef struct {
    int8_t value;
} KOS_Semaphore;

/**
 * Initializes a new semaphore
 */
KOS_Semaphore *kos_semaphore_init(int8_t value);

/**
 * Posts to a semaphore
 */
void kos_semaphore_post(KOS_Semaphore *sem);

/**
 * Pends from a semaphore
 */
void kos_semaphore_pend(KOS_Semaphore *sem);

#endif //KOS_SEMAPHORE

#ifdef KOS_QUEUE

typedef struct {
    void **messages;
    uint8_t pendIndex;
    uint8_t postIndex;
    uint8_t size;
} KOS_Queue;

/**
 * Initializes a new queue
 */
KOS_Queue *kos_queue_init(void **messages, uint8_t size);

/**
 * Posts to a queue
 */
void kos_queue_post(KOS_Queue *queue, void *message);

/**
 * Pends from a queue
 */
void *kos_queue_pend(KOS_Queue *queue);

#endif //KOS_QUEUE

/**
 * Runs the kernel
 */
void kos_run(void);

/**
 * Cooperative scheduling from task.
 */
void kos_yield(void);

/**
 * Cooperative scheduling from task, asking to sleep for at least 'delay' ms.
 */
void kos_delay_ms(uint16_t delay);

/**
 * When task will be schedule, it will wait for at least this delay.
 * Calling this function does not schedule the task, but it refers it's awake
 * time delay from the current tick timer value. This is intended to avoid shift
 * while the task treatment took some time.
 */
void kos_set_next_schedule_delay_ms(uint16_t delay);

/**
 * Intended to be the last call from a task
 */
void kos_task_exit();

/**
 * Runs the scheduler
 */
void kos_schedule(void);

/**
 * Increment tick and runs the scheduler
 * (called from timer ISR)
 */
void kos_tick_schedule(void);

/**
 * Dispatches the passed task, saving the context of the current task
 */
void kos_dispatch(KOS_Task *next);

#endif //_KOS_H_
