#include <libc.h>

char buff[24];

int pid;

char buffer[10000];
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

  	write(1, "write bien\n",11);
    
  	
    
  while(1) {
  	int a = gettime();
    itoa(a,buffer);
  	write(1,buffer,strlen(buffer));
  }
}
