#include <libc.h>

char buff[24];

int pid;
int zeos_ticks;

int addASM(int, int);

int zeos_ticks;
int write(int fd, char* buffer,int size);

int __attribute__ ((__section__(".text.main")))
  main(void)
{

	// int numero = addASM(0x42,0x666);
    
    //int* numero = 0;
  	//*numero = 0;

  	  write(1, "\nhola\n", 24);
      gettime();
  while(1) {


  }
}
