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

int pid_global = 2;

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
{
	return current()->PID;}

extern struct list_head freequeue;
extern struct list_head readyqueue;


struct task_struct * get_free_task_struct() {
	if (list_empty(&freequeue)) {
		return NULL;
	}

	struct list_head *next = list_first(&freequeue);
	list_del(next);
	return list_head_to_task_struct(next);
}


int ret_from_fork() {
	return 0;
}

int sys_fork(void) {
	struct task_struct *parent_task = current();

    struct task_struct *child_task = get_free_task_struct();

    if (child_task == NULL) {
        return -1; // No hay espacio para un nuevo proceso
    }

    // Copiar la estructura de task_union del padre al hijo
    copy_data(parent_task, child_task, sizeof(union task_union));

    // Asignar un nuevo directorio de páginas al proceso hijo
    allocate_DIR(child_task);

    // Obtener las tablas de páginas del padre e hijo
    page_table_entry *child_PT = get_PT(child_task);
    page_table_entry *parent_PT = get_PT(parent_task);

	int paginas[NUM_PAG_DATA];
	// Buscar y asignar frames físicos para mapear las páginas lógicas de datos y pila del hijo
    for (int i = 0; i < NUM_PAG_DATA; ++i) {

		paginas[i] = alloc_frame();	
        if (paginas[i] == -1) {
            // Liberar recursos y retornar error si no hay suficientes páginas
            for(int j = 0; j < i; j++) free_frame(paginas[j]);
            list_add_tail(&child_task->list,&freequeue);
			return -ENOMEM;
        }
    }



	for(int i = 0; i < NUM_PAG_KERNEL; i++){
		set_ss_pag(child_PT, i, get_frame(parent_PT, i));
	}

	for(int i = 0; i < NUM_PAG_CODE; i++){
		set_ss_pag(child_PT, PAG_LOG_INIT_CODE+i, get_frame(parent_PT, PAG_LOG_INIT_CODE+i));
	}

	
	// B) Apuntamos las nuevas páginas físicas a las direcciones lógicas del hijo.
	for(int i = 0; i < NUM_PAG_DATA; ++i){
		set_ss_pag(child_PT, PAG_LOG_INIT_DATA+i, paginas[i]);
	}

	int SHARED_SPACE = NUM_PAG_KERNEL+NUM_PAG_CODE;
	int TOTAL_SPACE = NUM_PAG_CODE+NUM_PAG_KERNEL+NUM_PAG_DATA;

	for(int i = SHARED_SPACE; i < TOTAL_SPACE; i++){
		set_ss_pag(parent_PT, i+NUM_PAG_DATA, get_frame(child_PT, i));
		// Las páginas estan alienadas a 4KB, por ese motivo se hace shift de 12bits para 
		// tener 3 ceros al final:
		copy_data((void *) (i << 12), (void *) ((i+NUM_PAG_DATA) << 12), PAGE_SIZE);
		del_ss_pag(parent_PT, i+NUM_PAG_DATA);
	}
	
	set_cr3(get_DIR(parent_task)); // Flush del TLB

	int pid_hijo= pid_global++;
	child_task->PID = pid_hijo;
	child_task->state=ST_READY;


    // Insertar el proceso hijo en la cola de procesos listos
    list_add_tail(&child_task->list, &readyqueue);

	// Configurar la pila del sistema del proceso hijo para la restauración del contexto
	union task_union * child_stack = (union task_union *) child_task;
	((unsigned long *)KERNEL_ESP(child_stack))[-0x13] = (unsigned long) 0; // Fake EBP
	((unsigned long *)KERNEL_ESP(child_stack))[-0x12] = (unsigned long) ret_from_fork; // @ret
	child_task->kernel_esp = (unsigned int)&((unsigned long *)KERNEL_ESP(child_stack))[-0x13];
    return pid_hijo;
}


void sys_exit()
{ 
	struct task_struct *current_task = current();

    // Free the physical memory used by the current process
    page_table_entry *PT = get_PT(current_task);

    for (int i = 0; i < NUM_PAG_DATA; ++i) {
        free_frame(get_frame(PT, i+PAG_LOG_INIT_DATA));
		del_ss_pag(PT, i+PAG_LOG_INIT_DATA);
    }

    current_task->PID = -1;
	current_task->dir_pages_baseAddr = NULL;
	update_process_state_rr(current_task, &freequeue);
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