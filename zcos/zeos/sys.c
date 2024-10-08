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

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

extern int zeos_ticks;

int sys_gettime(){
	return zeos_ticks;
}

char buffer_k[256];
#define BUFFER_SIZE 256

int sys_write(int fd, char * buffer, int size){

	// Si el valor es 1, es error.
	int fd_error = check_fd(fd, ESCRIPTURA);
	if(fd_error) return fd_error; // Si es error, retornem error (valor negatiu amb codi error).

	// Comprovem que el buffer sigui vàlid
	if(buffer == NULL) return -EFAULT; // EFAULT REPLACE
	if(size < 0) return -EINVAL; // EINVAL REPLACE

	int bytes = size;
	int written_bytes; 

	// Copiar los bytes que caben en el buffer
	while(bytes > BUFFER_SIZE){
		copy_from_user(buffer+(size-bytes), buffer_k, BUFFER_SIZE);
		written_bytes = sys_write_console(buffer_k, BUFFER_SIZE);
		
		buffer = buffer+BUFFER_SIZE;
		bytes = bytes-written_bytes;
	}

	// Copiar los bytes que sobran
	copy_from_user(buffer+(size-bytes), buffer_k, bytes);
	written_bytes = sys_write_console(buffer_k, bytes);
	bytes = bytes-written_bytes;	

	// Si no se han escrito todos los bytes, devolvemos error.
	return size-bytes;
}