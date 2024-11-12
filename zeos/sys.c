/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1


int base_PID = 905;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES;
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{   printk("\n");
	return current()->PID;
}

int ret_from_fork() {
    printk("La direccion real del task struct es:");
	print_number(current());
    printk("\n");
    
    return 0;
}

extern struct list_head freequeue;
extern struct list_head readyqueue;
extern struct list_head blocked;

struct task_struct* new;



void sys_exit()
{ 
	if(sys_getpid() == 1) return;
    struct task_struct* t = current();
    printk("El PID del proceso que se va a eliminar es:");
    print_number(t->PID);
    printk("\n");
	page_table_entry *entry = t->dir_pages_baseAddr;

	for (int i = 0; i < NUM_PAG_DATA; ++i) {
		free_frame(get_frame(entry, i + NUM_PAG_KERNEL));
		del_ss_pag(entry, i + NUM_PAG_KERNEL);
	}
	t->PID = -1;
	t->dir_pages_baseAddr = NULL;
    t->father = NULL;

    if (t->anchor.next == NULL && t->anchor.prev == NULL) list_del(&t->anchor);

    struct list_head * e = list_first(&(current()->childs));
    if (!list_empty(&(current()->childs))) {
        list_for_each(e, &(current()->childs)) {
            struct task_struct* ts = list_head_to_task_struct(e);
            ts->father = NULL;
            list_del(&ts->anchor);
        }
    }

	update_process_state_rr(t, &freequeue);
	sched_next_rr();
}

extern int zeos_ticks;

int sys_gettime(){
	return zeos_ticks;
}

char buffer_k[256];
#define BUFFER_SIZE 256


int sys_write(int fd, char *buffer, int size) {
    // Verificar si el descriptor de archivo es válido
    int fd_error = check_fd(fd, ESCRIPTURA);
    if (fd_error) return fd_error; // Si hay un error, retorna el código de error

    // Verificar si el buffer es válido
    if (buffer == NULL) return -EFAULT; // Reemplazo de EFAULT
    if (size < 0) return -EINVAL; // Reemplazo de EINVAL

    // Usar access_ok para verificar si el buffer de usuario es accesible
    if (!access_ok(VERIFY_WRITE, buffer, size)) {
        return -EFAULT; // Retorna error si el acceso a la memoria es inválido
    }

    int bytes = size;
    int written_bytes;

    // Copiar los bytes que caben en el buffer
    while (bytes > BUFFER_SIZE) {
        copy_from_user(buffer + (size - bytes), buffer_k, BUFFER_SIZE);
        written_bytes = sys_write_console(buffer_k, BUFFER_SIZE);

        buffer = buffer + BUFFER_SIZE;
        bytes = bytes - written_bytes;
    }

    // Copiar los bytes que sobran
    copy_from_user(buffer + (size - bytes), buffer_k, bytes);
    written_bytes = sys_write_console(buffer_k, bytes);
    bytes = bytes - written_bytes;

    // Si no se han escrito todos los bytes, devolvemos error.
    return size - bytes;
}


int sys_fork (void) {
    int PID = -1;

    printk("\n");
    printk("Esto es el fork:");
    printk("\n");
    printk("\n");

    if (!list_empty(&freequeue)) {
        struct list_head *lh = list_first(&freequeue);
        list_del(lh);
        struct task_struct *ts = list_head_to_task_struct(lh);

        copy_data(current(), ts, sizeof(union task_union));
        allocate_DIR(ts);

        int avail_frames[NUM_PAG_DATA];

        for (int i = 0; i < NUM_PAG_DATA; ++i) {
            avail_frames[i] = alloc_frame();
            if (avail_frames[i] < 0) {
                for (int j = 0; j <= i; ++j) {
                    free_frame(avail_frames[j]);
                }
                list_add_tail(&ts->list, &freequeue);
                return -1;
            }
        }

        for (int i = 0; i < NUM_PAG_KERNEL; ++i) {
            set_ss_pag(get_PT(ts), i, get_frame(get_PT(current()), i));
        }

        for (int i = 0; i < NUM_PAG_DATA; ++i) {
            set_ss_pag(get_PT(ts), i + NUM_PAG_KERNEL, avail_frames[i]);
            set_ss_pag(get_PT(current()), i + NUM_PAG_KERNEL + NUM_PAG_DATA + NUM_PAG_CODE, avail_frames[i]);
            copy_data((void*)((i + NUM_PAG_KERNEL) << 12), (void*)((i + NUM_PAG_KERNEL + NUM_PAG_DATA + NUM_PAG_CODE) << 12), PAGE_SIZE);
            del_ss_pag(get_PT(current()), i + NUM_PAG_KERNEL + NUM_PAG_DATA + NUM_PAG_CODE);
        }

        for (int i = 0; i < NUM_PAG_CODE; ++i) {
            set_ss_pag(get_PT(ts), i + NUM_PAG_KERNEL + NUM_PAG_DATA , get_frame(get_PT(current()), i + NUM_PAG_KERNEL + NUM_PAG_DATA ));
        }

        set_cr3(get_DIR(current()));

        

        PID = base_PID++;
        ts->PID = PID;
        printk("El PID del hijo es:");
        print_number(ts->PID);
        printk("\n");
        printk("El task struc del hijo es:");
        print_number(&ts);
        printk("\n");
        ts->father = current();
        ts->pending_unblocks = 0;
        INIT_LIST_HEAD(&ts->anchor);
        INIT_LIST_HEAD(&ts->childs);

        union task_union *tu = (union task_union*) ts;

        tu->stack[KERNEL_STACK_SIZE - 19] = (unsigned long) 0;

        tu->stack[KERNEL_STACK_SIZE - 18] = (unsigned long) ret_from_fork;

        ts->kernel_esp = (unsigned long) &tu->stack[KERNEL_STACK_SIZE - 19];

        list_add_tail(&ts->list, &readyqueue);

        ts->state = ST_READY;
        
        list_add_tail(&ts->anchor, &(current()->childs));
        
       

        new = ts;

        printk("\n");
        printk("Se acaba el fork:");
        printk("\n");
        printk("\n");

        return PID; 
    }
    
    return -1;
}

void sys_block(void) {
    current()->pending_unblocks = current()->pending_unblocks -1;
    if (current()-> father != NULL) {
        if (current()->pending_unblocks <= 0) {
            printk("ESTOY BLOQUEADO");
            printk("\n");
            update_process_state_rr(current(), &blocked);
            sched_next_rr();
        }
        else {
            printk("NO ESTOY BLOQUEADO");
            printk("\n");
        }
    }

}

int sys_unblock(int pid) {
    printk("\n");
    printk("He entrado en unblock\n");
    struct list_head *tmp = &(current()->childs);
    printk("La direccion de childs es:");
    print_number((int)tmp); // Imprimir dirección como entero
    printk("\n");
    printk("EMPTY:");
    print_number(list_empty(tmp));
    printk("\n");

    struct list_head *e = tmp->next;

    // Recorrer la lista manualmente
    while (e != tmp) {
        printk("El pid seleccionado es:");
        print_number(pid);
        printk("\n");

        struct task_struct* ts = list_entry(e, struct task_struct, anchor);
        printk("El task struct del hijo es:");
        print_number((int)ts); // Imprimir dirección como entero
        printk("\n");
        printk("El pid del hijo es:");
        print_number(ts->PID);
        printk("\n");
        printk("El state del hijo es:");
        print_number(ts->state);
        printk("\n");

        if (ts->PID == pid && ts->state == ST_BLOCKED) {
            printk("El hijo está bloqueado\n");
            update_process_state_rr(ts, &readyqueue);
            return 0;
        } else if (ts->PID == pid) {
            printk("El hijo no esta bloqueado\n");
            ts->pending_unblocks++;
        } else {
            printk("El hijo no esta\n");
        }

        e = e->next; // Avanzar al siguiente elemento
    }
    
    printk("He salido de unblock\n");
    printk("\n");
    return 0;

}