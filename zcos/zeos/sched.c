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
  return (struct task_struct *)((unsigned int)l);

  //return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

struct list_head freequeue;

struct list_head readyqueue;

struct task_struct * idle_task;

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

	while(1)
	{
	;
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

void inner_task_switch(union task_union*t) {

	// Set the kernel stack pointer to the kernel_esp of the task
	tss.esp0 = KERNEL_ESP(t);

	// Write the address of the stack to the model-specific register (MSR)
	write_msr(0x175, (int)tss.esp0);

	// Set the CR3 register to the base address of the page directory
	set_cr3(t->task.dir_pages_baseAddr);


	//La primera variable es %0, la segunda %1, la tercera %2, etc.
	
	//Guarda EBP al PCB
	__asm__ __volatile__(
		"pushl %%ebp\n\t"
		:
	);
	
	//Guarda ESP en EBP
	__asm__ __volatile__(
		"movl %%esp, %%ebp\n\t"
		:
	);


	// Cambiar valor de Kernel_ESP al ESP actual
	__asm__ __volatile__(
		"movl %%esp, %0\n\t"
		// "=g" indica que el operando entre paréntesis se usa como fuente
		: "=g" (task->task.kernel_esp)
		:
		);


	// Restaura EBP de la pila
	__asm__ __volatile__(
		"popl %%ebp\n\t"
		:
		:
	);

	// Salta a la dirección de retorno
	__asm__ __volatile__(
		"ret\n\t"
		:
		:
	);
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

