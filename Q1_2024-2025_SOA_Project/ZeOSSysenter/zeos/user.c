#include <libc.h>
char buff[24];

int pid;
int getKey(char* b);
char* sbrk(int size);
int SetColor(int color, int background);
int gotoXY(int posX, int posY);
int spritePut(int posX, int posY, Sprite* sp);
int threadCreate( void (*function)(void* arg), void* parameter );


void printnum(int num) {
    char buffer[12]; 
    int i = 0;

    if (num == 0) {
        buffer[i++] = '0';
    } else {
        while (num > 0) {
            buffer[i++] = (num % 10) + '0';  
            num /= 10;
        }
    }

    for (int j = i - 1; j >= 0; j--) {
        write(1, &buffer[j], 1); 
    }
}




int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  int pid2 = fork();

  if(pid2 == 0) {
    write(1,"holaaa",6);
    exit();
  }


    write(1,"\n",1);
    char* a = sbrk(4);
    if (a == (char*)-1) {
        
        write(1,"Error",5);
        return -1;
    }
    
    write(1,"El char en a vale:",18);
    *a = 'b';
    
    write(1, a, 1);
    write(1,"\n",1);

    int value = 42;
    *((int*)a) = value;

    int* ptr = (int*)a;  
    int stored_value = *ptr;

  int pid = fork();
  if(pid== 0) {
    write(1,"Soy el hijo\n",12);
    sbrk(0);
    write(1, "Stored value: ", 15); 
    printnum(stored_value);      
    write(1,"\n", 1);
    write(1,"\n",1);
    exit();
  } else {
    a = sbrk(0);
    a = sbrk(-4);
    a = sbrk(0);
    a = sbrk(-50000);
    a = sbrk(0);
    a = sbrk(5000);
    a = sbrk(0);
    a = sbrk(-50000);
    a = sbrk(0);
    a = sbrk(4097);
    a = sbrk(0);
    sbrk(100000);
    sbrk(0);
    sbrk(-5);
    sbrk(0);    
  }

    

  // char spriteContent[] = {
  //   '*', '*', '*', ' o', 'o', 'o', 'o', 'o', 'o', 'o', 
  //   '*', '*', '*',  '*', 'o', 'o', 'o', 'o', 'o', 'o',
  //   'z', 'z', 'z', 'z', 'z', 'o', 'o', '*', 'X', '|',
  //   '*', '*', '*', '*', '*', '*', '*', '*', 'X', '|',
  //   '|', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', '|',
  //   '|', '-', '-', '-', '-', '-', '-', '-', '-', '|',
  //   '|', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', '|',
  //   '-', '-', '-', '-', '-', '-', '-', '-', '-', '-'
  // };

  // Sprite sp = { 7, 10, spriteContent };
  // SetColor(7, 2);
  // gotoXY(10,10);
  // spritePut(0, 0, &sp);
  // gotoXY(5, 5);
  // SetColor(5, 2); 

  //char b;

  //threadCreate(111,0);

  while(1) {
    //if(getKey(&b) == 0) write(1,&b,1);
    
  }
}
