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

#include <list.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define COPY_PAGE TOTAL_PAGES - 1
#define USR_SK_START TOTAL_PAGES - 2

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
int global_TID=9050;

int ret_from_fork()
{
  return 0;
}

int seach_free_page (page_table_entry *PT) {
  for (int i = TOTAL_PAGES - 2; i >= 0; --i) {
    if (!PT[i].bits.present) return i;
  }
}

int isMaster() {
  if (current()->master == current()) return 1;
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

  ////("1");

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
//("2");

  
  int heap_pages = current()->heap_ptr?(current()->heap_ptr - (char*)(L_USER_START + (NUM_PAG_CODE + NUM_PAG_DATA) * PAGE_SIZE)) / PAGE_SIZE + 1 : 0;

  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, COPY_PAGE, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((COPY_PAGE)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, COPY_PAGE);
    set_cr3(get_DIR(current()));
  }   
   
//("3");

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
                list_add_tail(lhcurrent, &freequeue); //<== Te lo he anadido, creo que faltaba esto.
                return -EAGAIN;
            }
            set_ss_pag(parent_PT, COPY_PAGE, get_frame(process_PT, i));
            copy_data((void*)(i << 12), (void*)(COPY_PAGE << 12), PAGE_SIZE);
            del_ss_pag(parent_PT, COPY_PAGE);
        } 
        uchild->task.heap_ptr = heap_ptr;
  }else uchild->task.heap_ptr = NULL;
  set_cr3(get_DIR(current()));

//("4");


  char *usr_ptr = current()->user_stack_sp;

  // A tener en cuenta, el primer proceso tiene el usrStk dentro de la seccion de datos, como que es el unico que tiene PID = 1
  // No tenemos que alocatar ninguna pagina.
  if (current()-> TID != 1) { 
    int usrStackPhyPage;
    if ((usrStackPhyPage = alloc_frame()) < 0) {
      list_add_tail(lhcurrent, &freequeue);
      return -EAGAIN;
    }
    set_ss_pag(parent_PT, COPY_PAGE, usrStackPhyPage);
    copy_data((void*)usr_ptr, (void*)((COPY_PAGE << 12)), PAGE_SIZE);
    del_ss_pag(parent_PT, COPY_PAGE);
    set_ss_pag(process_PT, (int)usr_ptr >> 12, usrStackPhyPage);
    set_cr3(get_DIR(current()));
  }


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
  INIT_LIST_HEAD(&uchild->task.threads);
  uchild->task.master = &uchild->task;
  uchild->task.num_threads = 1;
  //("6");

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
  struct task_struct *master = current()->master;
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

  for (int i = 0; i < 10; ++i) {
    master->semfs[i].count = -1;
    master->semfs[i].TID = -1;
    INIT_LIST_HEAD(&master->semfs[i].blocked);
  }

  /* Free task_struct */

  if (!list_empty(&master->threads)) {
    struct list_head* e = list_first(&master->threads);
    while (e != &master->threads) {
      struct task_struct* ts = list_head_to_task_struct(e);
      free_frame(get_frame(get_PT(ts), ((int)(ts->user_stack_sp) >> 12)));
      del_ss_pag(get_PT(ts), ((int)(ts->user_stack_sp) >> 12));
      ts->TID = -1;
      ts->PID = -1;
      ts->num_threads = 0;
      list_del(&ts->list);
      list_add_tail(&ts->list, &freequeue);
      e = e->next;
    }
  }
  free_frame(get_frame(get_PT(master), ((int)(master->user_stack_sp) >> 12)));
  del_ss_pag(get_PT(master), ((int)(master->user_stack_sp) >> 12));
  master->TID = -1;
  master->PID = -1;
  list_add_tail(&master->list, &freequeue);
  master->num_threads = 0;
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

        ////(heap_ptr_actual);
        ////("\n");
    }

    if (size > 0) {

        //("\n");
        //("Operación sumar:");
        //(sumador);
        //("\n");

        int allocated_pages[128];
        int allocated_count = 0;
        while (sumador > 0) {
            //("Sumador:");
            //(sumador);
            //("\n");
            int available_space_in_current_page = PAGE_SIZE - (unsigned long)ts->heap_ptr % PAGE_SIZE;
            //("Espacio en pagina actual:");
            //(available_space_in_current_page);
            //("\n");
            if (sumador <= available_space_in_current_page) {
                //("Me cabe en la pagina\n");
                ts->heap_ptr += sumador;
                //("Heap futuro:");
                //(ts->heap_ptr);
                //("\n");
                return ts->heap_ptr - size;
            } else {
                //("Pido otra pagina\n");
                int new_ph_pag = alloc_frame();
                if (new_ph_pag == -1) {
                  //(allocated_count);
                  //("\n");
                  //("Fin de paginas\n");
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
            //("Error limite minimo de heap");
            return (char*)-1;
        }
        
        //("\n");
        //("Operación restar:");
        //(restador);
        //("\n");

        int prev_logical_page = (ts->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;
        //("Pagina logica actual:");
        //(prev_logical_page);
        //("\n");

        while (restador > 0) {
            int space_in_current_page = PAGE_SIZE - (unsigned long)ts->heap_ptr % PAGE_SIZE;
            //("Espacio en pagina actual:");
            //(space_in_current_page);
            //("\n");

            ts->heap_ptr -= restador;
            int current_logical_page = (ts->heap_ptr - (char*)L_USER_START) / PAGE_SIZE + NUM_PAG_KERNEL;
            //("Pagina logica futura:");
            //(current_logical_page);
            //("\n");

            //("Heap futuro:");
            //(ts->heap_ptr);
            //("\n");
            // comprobar que rango size no modifica otras secciones, limit del heap 0x11c 
            if(current_logical_page < 0x11c) {
                  //("Operación invalida\n");
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
    
    //("\n");
    //("HEAP:");
    //(heap_ptr_actual);
    //("\n");
    return heap_ptr_actual;
}

#define NUM_COLUMNS 80
#define NUM_ROWS    25

int sys_spritePut(int posX, int posY, Sprite* sp) {
    // Verificar que el puntero del sprite y su contenido son válidos
    if (sp == NULL || sp->content == NULL) {
        return -1; // Error si el sprite no es válido
    }

    // Verificar acceso a memoria del sprite
    if (!access_ok(VERIFY_READ, sp, sizeof(Sprite)) || 
        !access_ok(VERIFY_READ, sp->content, sp->x * sp->y)) {
        return -1; // Error si no es seguro acceder
    }

    // Asegurarse de que las posiciones están dentro del rango
    if (posX < 0 || posY < 0 || posX >= 80 || posY >= 25) {
        return -1; // Error si está fuera de la pantalla
    }

    for (int row = 0; row < sp->y; row++) { 
        for (int col = 0; col < sp->x; col++) { 
            int screenX = posX + col;
            int screenY = posY + row;
            if (screenX < 80 && screenY < 25 && screenX >= 0 && screenY >= 0) {
                char character = sp->content[row * sp->x + col]; 
                if (character != ' ') { 
                    printc_xy(screenX, screenY, character);
                }
            }
        }
    }

    return 0; // Éxito
}

extern Byte x,y;
extern int color_text,background_text;


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

    color_text = color;
    background_text = background;

    return 0;
}

int sys_threadCreate( void (*function)(void* arg), void* parameter, void* wrapper) {
  //("threadCreate");
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  struct task_struct *master = current()->master;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);

  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));

  uchild->task.TID = global_TID++;
  unsigned int userStackLogPage = (unsigned int)seach_free_page(get_PT(current()));

  int userStackPhyPage = alloc_frame();
  if (userStackPhyPage < 0) {
    return -EAGAIN;
  } else if (current()->heap_ptr?((int)(current()->heap_ptr) >> 12) >= userStackLogPage: 0) {
    free_frame(userStackPhyPage);
    return -ENOMEM;
  }

  set_ss_pag(get_PT(current()), userStackLogPage, userStackPhyPage);

  int* user_stack = (int*)(userStackLogPage << 12);
  user_stack[1023] = parameter;
  user_stack[1022] = function;
  user_stack[1021] = 0;

  uchild->stack[KERNEL_STACK_SIZE - 2] = (unsigned int) (&user_stack[1021]);
  uchild->stack[KERNEL_STACK_SIZE - 5] = (unsigned int) wrapper;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);
  uchild->task.register_esp=register_ebp;

  
  uchild->task.user_stack_sp = userStackLogPage << 12;
  uchild->task.state=ST_READY;
  init_stats(&(uchild->task.p_stats));

  list_add_tail(&(uchild->task.list), &readyqueue);
  list_add_tail(&(uchild->task.threads_list) ,&(master->threads));

  master->num_threads = master->num_threads + 1;

  
  return uchild->task.TID;
}

void sys_threadExit(void) {

  struct task_struct* master = current()->master;
  if (master->num_threads == 1) sys_exit();

  free_frame(get_frame(get_PT(current()), ((int)(current()->user_stack_sp) >> 12)));
  del_ss_pag(get_PT(current()), ((int)(current()->user_stack_sp) >> 12));
  current()->TID = -1;
  current()->PID = -1;

  //Hay que definir el nuevo master
  if(isMaster()) {
    struct list_head *lhcurrent = NULL;
    int found = 0;
    struct list_head * e = list_first(&master->threads);
    while (e != &master->threads && !found) {
      struct task_struct* ts = list_head_to_task_struct(e);
      if (ts->state != ST_BLOCKED) found = 1; 
      else e = e->next;
    }

    struct task_struct* newMaster = list_head_to_task_struct(e);
    newMaster->num_threads = master->num_threads - 1;
    for (int i = 0; i < 10; ++i) {
      newMaster->semfs[i].count = master->semfs[i].count;
      newMaster->semfs[i].TID = master->semfs[i].TID;
      INIT_LIST_HEAD(&newMaster->semfs[i].blocked);
      e = list_first(&master->semfs[i].blocked);
      while (e != &master->semfs[i].blocked) {
        list_add_tail(&e, &newMaster->semfs[i].blocked);
        e = e->next;
      }
 
    }

    list_del(&newMaster->threads_list);
    e = list_first(&master->threads);
    newMaster->master = newMaster;
    while (e != &master->threads) {
      list_add_tail(&e, &newMaster->threads);
      struct task_struct* ts = list_head_to_task_struct(e);
      ts->master = &newMaster;
      e = e->next;
    }
  }
  list_add_tail(&current()->list, &freequeue);
  sched_next_rr(); 
}

int sys_semCreate(int initial_value) {
  struct task_struct *master = current()->master;
  int i;
  for (i = 0; i < 10; ++i) {
    if (master->semfs[i].TID == -1) break;
  }

  if (i == 10) return -ENOMEM;

  master->semfs[i].count = initial_value;
  INIT_LIST_HEAD(&master->semfs[i].blocked);
  master->semfs[i].TID = current()->TID;

  return i;
}

int sys_semWait(int semid) {
  struct task_struct *master = current()->master;
  if (semid < 0 || semid > 9 || master->semfs[semid].TID == -1) return -EAGAIN;
  
  master->semfs[semid].count -= 1;
  if (master->semfs[semid].count < 0) {
    current()->state = ST_BLOCKED;
    list_add_tail(&current()->list,&master->semfs[semid].blocked);
    sched_next_rr();
  }
  return 0;
}

int sys_semSignal(int semid) {
  struct task_struct *master = current()->master;
  if (semid < 0 || semid > 9 || master->semfs[semid].TID == -1) return -EAGAIN;

  master->semfs[semid].count += 1;
  if (master->semfs[semid].count >= 0) {
      struct list_head *lhcurrent = NULL;
      if (list_empty(&master->semfs[semid].blocked)) return 0;
      lhcurrent=list_first(&master->semfs[semid].blocked);
      list_del(lhcurrent);
      struct task_struct* tu = (union task_union*)list_head_to_task_struct(lhcurrent);

      tu->state = ST_READY;
      list_add_tail(&tu->list, &readyqueue);
  }
  return 0;
}

int sys_semDestroy(int semid) {
  struct task_struct *master = current()->master;
  if (semid < 0 || semid > 9 || master->semfs[semid].TID != current()->TID) return -EAGAIN;
  
  while (!list_empty(&master->semfs[semid].blocked)) {
      struct list_head *lhcurrent = NULL;
      if (list_empty(&master->semfs[semid].blocked)) return 0;
      lhcurrent=list_first(&master->semfs[semid].blocked);
      list_del(lhcurrent);
      struct task_struct* tu = list_head_to_task_struct(lhcurrent);

      tu->state = ST_READY;
      list_add_tail(&tu->list, &readyqueue);
  }
  master->semfs[semid].count = -1;
  master->semfs[semid].TID = -1;
  return 0;
}

void sys_clean_screen() {
  int posX,posY = 0;
  for (int i = 0; i < 25; ++i) {
    for (int j = 0; j < 80; ++j ) {
      if(posX+j < 80 && posY+i < 25) {
        char temp_c = ' ';
        printc_xy(posX+j,posY+i,temp_c);
      }
    }
  }
}

void sys_clean_region(int startX, int startY, int endX, int endY) {
    for (int i = startY; i <= endY; ++i) {
        for (int j = startX; j <= endX; ++j) {
            if (j >= 0 && j < 80 && i >= 0 && i < 25) { 
                char temp_c = ' ';
                printc_xy(j, i, temp_c);
            }
        }
    }
}