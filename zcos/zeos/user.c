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

  write(1, "Ejemplo write\n",14);

  
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
