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
		struct list_head *siguiente = list_first(&freequeue);
		list_del(siguiente);
		struct task_struct * nuevo_task_struck = list_head_to_task_struct(siguiente);
		nuevo_task_struck->PID = 0;
		allocate_DIR(nuevo_task_struck);
		union task_union* libre = (union task_union*) nuevo_task_struck;
		libre->stack[KERNEL_STACK_SIZE - 1] = (unsigned long) cpu_idle;
		libre->stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0;
		nuevo_task_struck->kernel_esp = (unsigned int)&libre->stack[KERNEL_STACK_SIZE - 2];
		idle_task = nuevo_task_struck; 
	}


}

void init_task1(void)
{

	if(!list_empty(&freequeue)) {
		struct list_head *siguiente = list_first(&freequeue);
		list_del(siguiente);
		struct task_struct * nuevo_task_struck = list_head_to_task_struct(siguiente);
		nuevo_task_struck->PID = 1;
		allocate_DIR(nuevo_task_struck);
		set_user_pages(nuevo_task_struck);



		union task_union* libre = (union task_union*) nuevo_task_struck;
		write_msr(0x175,(unsigned int)&libre->stack);
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

	t->

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

