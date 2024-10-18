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
  return (struct task_struct *)((unsigned int)l & 0xfffff000);

  //return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

struct list_head freequeue;

struct list_head readyqueue;

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

}

void init_task1(void)
{
}


void init_sched()
{
	freequeue = task[0].task.list;
	struct list_head aux = freequeue;

	for (int i = 1; i < NR_TASKS; ++i)
	{
		list_add_tail()
	}

	/*
	for (int i = 0; i < NR_TASKS; i++)
	{
		//idle process
		if(i == 0) {

			freequeue = task[0].task.list;
			aux = freequeue;
		}
		else {
			//ir metiendo los procesos
			aux.next = &task[i].task.list;
			aux = *aux.next;
			aux.prev = &task[i-1].task.list;
			 	
		}
		
	}
	aux.next = &task[0].task.list;
	*/
	//inicializar readyqueue
	INIT_LIST_HEAD(&readyqueue);
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

