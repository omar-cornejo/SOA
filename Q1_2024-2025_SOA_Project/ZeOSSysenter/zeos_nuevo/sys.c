/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }

  
  int heap_pages = 0;
  if(current()->heap_ptr != NULL) {
    heap_pages = (current()->heap_ptr - (char*)(L_USER_START + (NUM_PAG_CODE + NUM_PAG_DATA) * PAGE_SIZE)) / PAGE_SIZE + 1;
    if((TOTAL_PAGES - 1 - (((unsigned long)current()->heap_ptr)>>12)) < NUM_PAG_DATA) {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      return -ENOMEM;
    }
    for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
    {
      /* Map one child page to parent's address space. */
      set_ss_pag(parent_PT, pag+NUM_PAG_DATA+heap_pages, get_frame(process_PT, pag));
      copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA+heap_pages)<<12), PAGE_SIZE);
      del_ss_pag(parent_PT, pag+NUM_PAG_DATA+heap_pages);
    }   
  } else {
      /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
    for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
    {
      /* Map one child page to parent's address space. */
      set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
      copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
      del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
    }
  }

  set_cr3(get_DIR(current()));
  char *heap_ptr = current()->heap_ptr;

  if (heap_ptr != NULL) {
        int initial_logical_page = NUM_PAG_DATA + NUM_PAG_CODE + NUM_PAG_KERNEL;
        int current_logical_page = (current()->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;
        for (int i = initial_logical_page; i < current_logical_page + 1; i++) {
            int new_frame = alloc_frame();
            if (new_frame != -1) {
                set_ss_pag(process_PT, i, new_frame);
            } else {
                for (int j = initial_logical_page; j < i + 1; j++) {
                    free_frame(get_frame(process_PT, j));
                    del_ss_pag(process_PT, j);
                }
                return -EAGAIN;
            }
            set_ss_pag(parent_PT, i + heap_pages, get_frame(process_PT, i));
            copy_data((void*)(i << 12), (void*)((i + heap_pages) << 12), PAGE_SIZE);
            del_ss_pag(parent_PT, i + heap_pages);
        } 
        uchild->task.heap_ptr = heap_ptr;
  }else uchild->task.heap_ptr = NULL;
  
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		
    return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
    copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  if (current()->heap_ptr != NULL) {

        int initial_logical_page = NUM_PAG_DATA + NUM_PAG_CODE + NUM_PAG_KERNEL;
        int current_logical_page = (current()->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;

        while (initial_logical_page < current_logical_page + 1) {
            int frame = get_frame(process_PT, initial_logical_page);
            free_frame(frame);
            del_ss_pag(process_PT, initial_logical_page);
            initial_logical_page++;
        }
        
  }

  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}


#define BUFFER_SIZE 128
extern char circular_buffer[];
extern int write_pointer;
extern int read_pointer;
extern int buffer_count;
extern char char_map[];

int sys_getKey(char *b) {
    if (buffer_count > 0) {
        unsigned char scan_code = circular_buffer[read_pointer];
        read_pointer = (read_pointer + 1) % BUFFER_SIZE;
        buffer_count--;
        unsigned char scancode = scan_code & 0x7F;
        *b = char_map[scancode];
        return 0;
    }
    return -1;
}

char* sys_sbrk(int size) {
    struct task_struct* ts = current();
    char* heap_ptr_actual = ts->heap_ptr;
    int sumador = 0;
    int restador = 0;

    if (size < 0) {
        restador = -size;
    } else {
        sumador = size;
    }

    if(size == 0 && heap_ptr_actual == NULL) return (char*)-1;

    if (heap_ptr_actual == NULL) {
        int new_ph_pag = alloc_frame();
        if (new_ph_pag == -1) {
            return (char*)-1;
        }

        page_table_entry *process_PT = get_PT(ts);
        set_ss_pag(process_PT, NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA, new_ph_pag);
        ts->heap_ptr = (char*) (L_USER_START + (NUM_PAG_CODE + NUM_PAG_DATA) * PAGE_SIZE);
        heap_ptr_actual = ts->heap_ptr;

        //printnum(heap_ptr_actual);
        //printk("\n");
    }

    if (size > 0) {

        printk("\n");
        printk("Operación sumar:");
        printnum(sumador);
        printk("\n");

        int allocated_pages[128];
        int allocated_count = 0;
        while (sumador > 0) {
            printk("Sumador:");
            printnum(sumador);
            printk("\n");
            int available_space_in_current_page = PAGE_SIZE - (unsigned long)ts->heap_ptr % PAGE_SIZE;
            printk("Espacio en pagina actual:");
            printnum(available_space_in_current_page);
            printk("\n");
            if (sumador <= available_space_in_current_page) {
                printk("Me cabe en la pagina\n");
                ts->heap_ptr += sumador;
                printk("Heap futuro:");
                printnum(ts->heap_ptr);
                printk("\n");
                return ts->heap_ptr - size;
            } else {
                printk("Pido otra pagina\n");
                int new_ph_pag = alloc_frame();
                if (new_ph_pag == -1) {
                  printnum(allocated_count);
                  printk("\n");
                  printk("Fin de paginas\n");
                  //devolver todas las paginas demás
                  page_table_entry* process_PT = get_PT(ts);
                  for (int i = 0; i < allocated_count; i++) {
                        free_frame(get_frame(process_PT,allocated_pages[i]));
                        process_PT[allocated_pages[i]].entry = 0;
                        del_ss_pag(process_PT, allocated_pages[i]);
                  }
                  
                  ts->heap_ptr -= PAGE_SIZE*allocated_count;
                   
                  return (char*)-1;
                }

                page_table_entry *process_PT = get_PT(ts);
                int current_logical_page = (ts->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;
                set_ss_pag(process_PT, current_logical_page + 1, new_ph_pag);
                ts->heap_ptr += PAGE_SIZE;
                allocated_pages[allocated_count++] = new_ph_pag;
                sumador -= PAGE_SIZE;

            }
        }
        
        
    } else if (size < 0) {
        if (ts->heap_ptr == (char*) (L_USER_START + (NUM_PAG_CODE + NUM_PAG_DATA) * PAGE_SIZE)) {
            printk("Error limite minimo de heap");
            return (char*)-1;
        }
        
        printk("\n");
        printk("Operación restar:");
        printnum(restador);
        printk("\n");

        int prev_logical_page = (ts->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;
        printk("Pagina logica actual:");
        printnum(prev_logical_page);
        printk("\n");

        while (restador > 0) {
            int space_in_current_page = PAGE_SIZE - (unsigned long)ts->heap_ptr % PAGE_SIZE;
            printk("Espacio en pagina actual:");
            printnum(space_in_current_page);
            printk("\n");

            ts->heap_ptr -= restador;
            int current_logical_page = (ts->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;
            printk("Pagina logica futura:");
            printnum(current_logical_page);
            printk("\n");

            printk("Heap futuro:");
            printnum(ts->heap_ptr);
            printk("\n");
            // comprobar que rango size no modifica otras secciones, limit del heap 0x11c 
            if(current_logical_page < 0x11c) {
                  printk("Operación invalida\n");
                  ts->heap_ptr += restador;
                  return (char*)-1;
            }

            if (restador <= space_in_current_page) {
                return ts->heap_ptr + restador;
            } else {
                page_table_entry* process_PT = get_PT(ts);
                int frame = process_PT[prev_logical_page].bits.pbase_addr;
                free_frame(frame);
                process_PT[prev_logical_page].entry = 0;

                restador -= space_in_current_page;
                prev_logical_page = current_logical_page;
            }
        }
    }
    
    printk("\n");
    printk("HEAP:");
    printnum(heap_ptr_actual);
    printk("\n");
    return heap_ptr_actual;
}

int sys_spritePut(int posX, int posY, Sprite* sp) {
    if (sp == NULL || sp->content == NULL) {
        return -1; 
    }


    for (int row = 0; row < sp->x; row++) {
        for (int col = 0; col < sp->y; col++) {
            char character = sp->content[row * sp->y + col];  
            printc_xy(posX + col, posY + row, character); 
        }
    }

    return 0; 
}

extern Byte x,y;
#define NUM_COLUMNS 80
#define NUM_ROWS    25

int sys_gotoXY(int posX, int posY) {
    if (posX < 0 || posX >= NUM_COLUMNS || posY < 0 || posY >= NUM_ROWS) {
        return -1; 
    }

    x = posX;
    y = posY;

    return 0; 
}

int sys_SetColor(int color, int background) {

    if (color < 0 || color > 15 || background < 0 || background > 15) {
        return -1; 
    }

  
    char colorAttribute = (background << 4) | (color & 0x0F);

    Word *screen = (Word *)0xb8000;
    screen[(y * NUM_COLUMNS + x)] = (screen[(y * NUM_COLUMNS + x)] & 0x00FF) | (colorAttribute << 8);

    return 0;
}