/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{	
    return (struct task_struct *)((unsigned int)l - (unsigned int)&(((struct task_struct *)0)->list));
  //return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

struct list_head freequeue;

struct list_head readyqueue;

struct task_struct * idle_task;

struct task_struct * init_task;

void write_msr(unsigned long register, unsigned long address);


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	printk("idle\n");
	task_switch((union task_union *)init_task);
	while(1)
	{
	;
	}
	 
}

void update_sched_data_rr(void) {
    struct task_struct *current_task = current();
    current_task->quantum--;

    if (current_task->quantum <= 0) {
        // Quantum expired, scheduling decision needed
        current_task->quantum = 0; // Ensure it doesn't go negative
    }
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue) {
    if (t->state != ST_RUN) {
        list_del(&t->list); // Remove from current queue if not running
    }

    if (dst_queue != NULL) {
        list_add_tail(&t->list, dst_queue); // Add to new queue
    }

    // Update state based on the destination queue
    if (dst_queue == &readyqueue) {
        t->state = ST_READY;
    } else if (dst_queue == &freequeue) {
        t->state = ST_FREE;
    } else {
        t->state = ST_BLOCKED;
    }
}

int needs_sched_rr(void) {
    struct task_struct *current_task = current();

    // Check if the current process's quantum has expired
    if (current_task->quantum == 0) {
        return 1; // Context switch needed
    }

    return 0; // No context switch needed
}


void sched_next_rr(void) {
    if (list_empty(&readyqueue)) {
        // No other process to schedule, switch to idle
        task_switch((union task_union *)idle_task);
    } else {
        // Select the next process from the ready queue
        struct list_head *next_ready = list_first(&readyqueue);
        struct task_struct *next_task = list_head_to_task_struct(next_ready);
        list_del(next_ready); // Remove the process from the ready queue

        next_task->state = ST_RUN; // Set the new process state to running

        // Restore the process's quantum before switching
        set_quantum(next_task, DEFAULT_QUANTUM);

        // Perform context switch
        task_switch((union task_union *)next_task);
    }
}


int get_quantum(struct task_struct *t) {
    return t->quantum; // Devuelve el quantum de la tarea
}

void set_quantum(struct task_struct *t, int new_quantum) {
    t->quantum = new_quantum; // Modifica el quantum de la tarea
}

void schedule(void) {
    // Update the scheduling data for the current process
    update_sched_data_rr();

    // Check if a context switch is needed
    if (needs_sched_rr()) {
        // Update the state of the current process
        struct task_struct *current_task = current();
        if (current_task != idle_task) {
            // Move the current process to the end of the ready queue
            update_process_state_rr(current_task, &readyqueue);
        }

        // Select the next process to run and perform a context switch
        sched_next_rr();
    }
}

void init_idle (void)
{
	if(!list_empty(&freequeue)) {
		
		// Get the first element from the freequeue
		struct list_head *siguiente = list_first(&freequeue);
		
		// Remove the element from the freequeue
		list_del(siguiente);
		
		// Convert the list_head to a task_struct
		struct task_struct * nuevo_task_struck = list_head_to_task_struct(siguiente);
		
		// Set the PID of the new task to 0 (idle task)
		nuevo_task_struck->PID = 0;
		nuevo_task_struck->quantum = 0;
		
		// Allocate a page directory for the new task
		allocate_DIR(nuevo_task_struck);
		
		// Cast the task_struct to a task_union
		union task_union* libre = (union task_union*) nuevo_task_struck;
		
		// Set the last entry in the stack to point to the cpu_idle function
		libre->stack[KERNEL_STACK_SIZE - 1] = (unsigned long) cpu_idle;
		
		// Set the second to last entry in the stack to 0
		libre->stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0;
		
		// Set the kernel_esp to point to the second to last entry in the stack
		libre-> task.kernel_esp = (unsigned long) &libre->stack[KERNEL_STACK_SIZE - 2];
		
		// Set the idle_task pointer to the new task
		idle_task = nuevo_task_struck; 
	}


}

void init_task1(void)
{

	if(!list_empty(&freequeue)) {
		// Get the first element from the freequeue
		struct list_head *siguiente = list_first(&freequeue);
		
		// Remove the element from the freequeue
		list_del(siguiente);
		
		// Convert the list_head to a task_struct
		struct task_struct * nuevo_task_struck = list_head_to_task_struct(siguiente);
		
		// Set the PID of the new task to 1 (task 1)
		nuevo_task_struck->PID = 1;
		nuevo_task_struck->quantum = 0;
		
		// Allocate a page directory for the new task
		allocate_DIR(nuevo_task_struck);
		
		// Set up user pages for the new task
		set_user_pages(nuevo_task_struck);
		
		// Cast the task_struct to a task_union
		union task_union* libre = (union task_union*) nuevo_task_struck;
		
		// Set the last entry in the stack to point to the cpu_idle function
		tss.esp0 = KERNEL_ESP(libre);

		// Write the address of the stack to the model-specific register (MSR)
		write_msr(0x175, (int)tss.esp0);
		
		// Set the CR3 register to the base address of the page directory
		set_cr3(nuevo_task_struck->dir_pages_baseAddr);

	
	init_task = nuevo_task_struck;	
	}

	

}


void init_sched()
{	
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	for (int i = 0; i < NR_TASKS; ++i)
	{
		list_add_tail(&task[i].task.list, &freequeue);
		//(list) -> <- . . .  new ->  <- (list)
	}

	
}

void inner_task_switch(union task_union*new_task) {	
	
	// Establece el valor del registro esp0 del TSS con el puntero de pila del nuevo proceso en modo kernel
	tss.esp0 = KERNEL_ESP(new_task); 

	// Escribe el valor de esp0 en el MSR (Model-Specific Register) con el identificador 0x175
	write_msr(0x175, (unsigned int) tss.esp0);

	// Cambia el directorio de páginas al del nuevo proceso
	set_cr3(new_task->task.dir_pages_baseAddr);

	// Guarda el valor de la pila del proceso actual en la estructura task_struct
	unsigned int current_ebp = store_ebp_in_pcb();
	
	current()->kernel_esp = current_ebp;
	
	//Change the current system stack by setting ESP register to point to the stored value in the new PCB.
	change_stack(new_task->task.kernel_esp);
	
}



struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

