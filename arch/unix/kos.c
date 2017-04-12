/**
 * Main code for KOS
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "kos.h"
#include "kos_settings.h"
#include "msched.h"

/* Start & end of stack markers for overflow detection */
#define STACK_MAGIC1	0x11
#define STACK_MAGIC2	0x22
#define STACK_MAGIC3	0x33

/**
 * Context frame format:
 *
 * MAGIC1
 * MAGIC2
 * MAGIC3
 * PC_LOW
 * PC_HIGH
 * 3-byte PC (0x00)
 * r0-31
 * SREG
 */

static KOS_Task tasks[KOS_MAX_TASKS + 1];
static uint8_t tasks_count = 0;
static KOS_Task *task_head;
static KOS_Task *task_tail;
KOS_Task *kos_current_task;

static void task_idle(void)
{
	for(;;) {
		pause();
	}
}

#define TASK_IDLE_STACK		128

void kos_init(void)
{
	kos_new_task(task_idle, "IDLE", TASK_IDLE_STACK);
}

void kos_new_task(KOS_TaskFn task, const char *name, uint16_t size)
{
    uint8_t *stack;
    KOS_Task *tcb;

    stack = mmap(NULL,size,PROT_WRITE|PROT_READ,
		MAP_PRIVATE|MAP_GROWSDOWN|MAP_ANONYMOUS,-1,0);
    if (stack == MAP_FAILED) {
        fprintf(stderr, "kos_new_task: mmap failed with error %u\n", errno);
	goto kos_new_task_map_failed;
    }

#if 0
    //make space for pc, sreg, and 32 register
    stack[0]  = STACK_MAGIC1;
    stack[-1] = STACK_MAGIC2;
    stack[-2] = STACK_MAGIC3;
    stack[-3] = (uint16_t)task & 0xFF;
    stack[-4] = (uint16_t)task >> 8;
    stack[-5] = 0x00;
    for (i = -6; i > -38; i--)
    {
        stack[i] = 0;
    }
    stack[-38] = 0x80; //sreg, interrupts enabled
#endif

    /* TODO: check task_count != KOS_MAX_TASKS */
    //create the task structure
    tcb = &tasks[tasks_count++];
    //tcb->sp = stack - 39;
    tcb->status = TASK_READY;

    //insert into the task list as the new highest priority task
    if (task_head)
    {
        tcb->next = task_head;
        tcb->uctx.uc_link = &task_head->uctx;
        task_head = tcb;
        task_tail->next = tcb;
        task_tail->uctx.uc_link = &tcb->uctx;
    }
    else
    {
        task_head = tcb;
        task_tail = tcb;
    }

    tcb->stack_bottom = stack;
    tcb->stack_top = &stack[size-1];

#if 0
    tcb->stack_bottom[0] = STACK_MAGIC1;
    tcb->stack_bottom[1] = STACK_MAGIC2;
    tcb->stack_bottom[2] = STACK_MAGIC3;
#endif

    strncpy(tcb->name, name, TASK_NAME_MAXLEN);

    getcontext(&tcb->uctx);
    if (task) {
	    tcb->uctx.uc_stack.ss_sp = (void *) tcb->stack_top;
	    tcb->uctx.uc_stack.ss_size = size-1;
	    makecontext(&tcb->uctx, task, 0);
    }

    return;

kos_new_task_map_failed:
    exit(errno);
}

static uint8_t kos_isr_level = 0;
void kos_isr_enter(void)
{
    kos_isr_level++;
}

void kos_isr_exit(void)
{
    kos_isr_level--;
    kos_schedule();
}

#ifdef KOS_SEMAPHORE

static KOS_Semaphore semaphores[KOS_MAX_SEMAPHORES + 1];
static uint8_t next_semaphore = 0;

KOS_Semaphore *kos_semaphore_init(int8_t value)
{
    KOS_Semaphore *s = &semaphores[next_semaphore++];
    s->value = value;
    return s;
}

void kos_semaphore_post(KOS_Semaphore *semaphore)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        KOS_Task *task;
        semaphore->value++;

        //allow one task to be resumed which is waiting on this semaphore
        task = task_head;
        while (task)
        {
            if (task->status == TASK_SEMAPHORE && task->status_pointer == semaphore)
                break; //this is the task to be restored
            task = task->next;
        }

        task->status = TASK_READY;
        kos_schedule();
    }
}

void kos_semaphore_pend(KOS_Semaphore *semaphore)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        int8_t val = semaphore->value--; //val is value before decrement

        if (val <= 0)
        {
            //we need to wait on the semaphore
            kos_current_task->status_pointer = semaphore;
            kos_current_task->status = TASK_SEMAPHORE;

            kos_schedule();
        }
    }
}

#endif //KOS_SEMAPHORE

#ifdef KOS_QUEUE

#define NEXT_INDEX(I,S) ((I) < ((S) - 1) ? (I) + 1 : 0)

static KOS_Queue queues[KOS_MAX_QUEUES + 1];
static uint8_t next_queue = 0;

KOS_Queue *kos_queue_init(void **messages, uint8_t size)
{
    KOS_Queue *queue = &queues[next_queue++];

    queue->messages = messages;
    queue->pendIndex = queue->postIndex = 0;
    queue->size = size;

    return queue;
}

void kos_queue_post(KOS_Queue *queue, void *message)
{
    KOS_Task *task;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        queue->messages[queue->postIndex] = message;
        queue->postIndex = NEXT_INDEX(queue->postIndex, queue->size);

        task = task_head;
        while (task)
        {
            if (task->status == TASK_QUEUE && task->status_pointer == queue)
                break; //this is the task to be restored
        }
        task->status = TASK_READY;
        kos_schedule();
    }
}

void *kos_queue_pend(KOS_Queue *queue)
{
    void *data;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (queue->pendIndex == queue->postIndex)
        {
            //queue is empty, wait for next item
            kos_current_task->status_pointer = queue;
            kos_current_task->status = TASK_QUEUE;
            kos_schedule();
        }
        data = queue->messages[queue->pendIndex];
        queue->pendIndex = NEXT_INDEX(queue->pendIndex, queue->size);
    }
    
    return data;
}

#endif //KOS_QUEUE

#ifdef CONFIG_KOS_CHECK_STACKS
#define WAIT_FOREVER() do { cli(); for(;;) ; } while(0)

//static void dump_all_stack()
//{
//	uint8_t i;
//	KOS_Task *task;
//
//	printf("\n\n*** STACK OVERFLOW ***\n\n");
//	for (i = 0; i < tasks_count; i++) {
//		task = &tasks[i];
//
////		printf("%s:\tsize = %4d\t[0x%04x ... 0x%04x]\n",
////			task->name,
////			task->stack_top - task->stack_bottom + 1,
////			(unsigned int)task->stack_bottom,
////			(unsigned int)task->stack_top);
//	}
//}

static void stack_check_consistency()
{
#if 0
	if (kos_current_task) {

		uint16_t current_sp;
		uint8_t lsb;
		uint8_t msb;

		__asm__ volatile (
			"in %[_LSB_], %[_SPL_] \n\t"
			"in %[_MSB_], %[_SPH_] \n\t"
			:
			[_LSB_] "=r" (lsb),
			[_MSB_] "=r" (msb)
			:
			[_SPL_] "i" _SFR_IO_ADDR(SPL),
			[_SPH_] "i" _SFR_IO_ADDR(SPH)
		);

		current_sp = (uint16_t)msb << 8;
		current_sp |= lsb & 0x00ff;

		/* check if we have enough space for context switch */
		if (current_sp - 39 < (int)kos_current_task->stack_bottom) {
			dump_all_stack();
			printf("\ntask '%s' stack is too small!\n",
				kos_current_task->name);
			printf("current SP - 39 = 0x%04x\n", current_sp - 39);
			printf("requires %d more bytes at least\n",
				(int)kos_current_task->stack_bottom
				- (current_sp - 39));

			WAIT_FOREVER();
		}

		/* check if current stack did not have already overflowed */
		if (kos_current_task->stack_bottom[0] != STACK_MAGIC1 ||
		    kos_current_task->stack_bottom[1] != STACK_MAGIC2 ||
		    kos_current_task->stack_bottom[2] != STACK_MAGIC3) {

			dump_all_stack();
			printf("\ntask '%s' stack had overflowed.\n",
				kos_current_task->name);

			WAIT_FOREVER();
		}
	}
#endif
}
#endif /* CONFIG_KOS_CHECK_STACKS */

KOS_Task * kos_get_next_task(void)
{
	KOS_Task *t;
	KOS_Task *next_task = NULL;

	/* first scheduler call */
	if (!kos_current_task)
		return task_head;

	/* round-robin scheduling */
	for (t = kos_current_task->next; t != kos_current_task; t = t->next) {

		if (t->status == TASK_READY && !t->nb_tick_to_wait) {

			/* idle task is handled afterward */
			if (t == &tasks[0])
				continue;

			next_task = t;
			break;
		}
		else if ((t->status == TASK_ZOMBIE) && (t->uctx.uc_stack.ss_size != 0))
		{
			if (munmap(t->stack_bottom, t->uctx.uc_stack.ss_size))
			{
			        fprintf(stderr, "kos_get_next_task: munmap failed with error %u\n", errno);
			        exit(1);
			}

			t->uctx.uc_stack.ss_size = 0;
			t->uctx.uc_stack.ss_sp = NULL;
			t->stack_bottom = NULL;
			t->stack_top = NULL;
		}

	}

	return next_task; // ? next_task : kos_current_task;
}

void kos_run(void)
{
	kos_schedule();
}

void kos_yield(void)
{
	/* at least, a task will sleep one tick */
	if(!kos_current_task->nb_tick_to_wait)
		kos_current_task->nb_tick_to_wait++;
	kos_schedule();
}

void kos_delay_ms(uint16_t delay)
{
	kos_current_task->nb_tick_to_wait = delay / msched_get_tickms();
	kos_schedule();
}

void kos_set_next_schedule_delay_ms(uint16_t delay)
{
	kos_current_task->nb_tick_to_wait = delay / msched_get_tickms();
}

void kos_task_exit()
{
	kos_current_task->status = TASK_ZOMBIE;
	kos_schedule();

	/* should never go there */
	for(;;) ;
}

void kos_schedule(void)
{
	KOS_Task *task;

	if (kos_isr_level)
		return;

#ifdef CONFIG_KOS_CHECK_STACKS
	stack_check_consistency();
#endif

	task = kos_get_next_task();

	if (!task) {
		/* no active task: let's enter sleep mode with idle task */
		task = &tasks[0];
	}

	if (task != kos_current_task)
	{
		//ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			kos_dispatch(task);
		}
	}
}

void kos_tick_schedule(void)
{
	uint8_t i;
	KOS_Task *t;

	/* Note: not required, we are in ISR context */
	/*ATOMIC_BLOCK(ATOMIC_RESTORESTATE)*/
	{
		/* update waiting tasks */
		for (i = tasks_count; i; i--) {
			t = &tasks[i - 1];

			if (t->status == TASK_READY && t->nb_tick_to_wait)
				t->nb_tick_to_wait--;
		}
	}

	kos_schedule();
}

void kos_dispatch(KOS_Task *task)
{
	if (!kos_current_task) {
		kos_current_task = task;
		setcontext(&task->uctx);
	} else {
		KOS_Task * current = kos_current_task;

		//fprintf(stderr, "a:%s -> %s\n", kos_current_task->name, task->name);

		kos_current_task = task;
		swapcontext(&current->uctx, &task->uctx);
	}
}

