#include <libc.h>

char buff[4];

int pid;

int addASM(int, int);

int zeos_ticks;
int write(int fd, char* buffer,int size);
int gettime();

int __attribute__ ((__section__(".text.main")))
  main(void)
{

	// int numero = addASM(0x42,0x666);
    
  //int* numero = 0;
  //*numero = 0;


  int bytes;

  bytes = write(1, "Ejemplo write\n",14);
  if(bytes == -1) perror();

  bytes = write(1, "Ahora muestro mi pid: ",22);
  if(bytes == -1) perror();

  int pid = getpid();
  itoa(pid,buff);
  bytes = write(1,buff,strlen(buff));
  if(bytes == -1) perror();
  
  
  
  while(1) {
    
    /*
    int a = gettime();  
    char *puntero = buff;
    itoa(a,puntero);
    if((write(1,"gettime:",8)) == -1) perror();
    if((write(1,buff,strlen(buff))) == -1) perror();
    if((write(1,"\n",1)) == -1) perror();
    */
  }
} 
