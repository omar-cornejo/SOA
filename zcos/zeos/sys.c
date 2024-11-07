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
	return current()->PID;
}

extern struct list_head freequeue;



void sys_exit()
{  
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