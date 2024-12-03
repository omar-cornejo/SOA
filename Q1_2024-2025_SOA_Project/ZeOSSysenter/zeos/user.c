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


      

    write(1,"\n",1);
    
    //fork();
    
    char* a = sbrk(4);
    if (a == (char*)-1) {
        
        write(1,"Error",5);
        return -1;
    }
    sbrk(0);
    
    write(1,"El char en a vale:",18);
    *a = 'b';
    
    write(1, a, 1);
    write(1,"\n",1);

    sbrk(0);

  int value = 42;
  *((int*)a) = value;
  int* ptr = (int*)a;  
  int stored_value = *ptr;
  printnum(stored_value);   
  write(1,"\n",1);


  int pid = fork();
  if(pid== 0) {
    sbrk(0);
    printnum(stored_value);   
    sbrk(0);       
    exit();
  } 
  else {
    sbrk(0);
    sbrk(-4);
    sbrk(0);
    sbrk(-50000);
    sbrk(0);
    sbrk(5000);
    sbrk(0);
    sbrk(-50000);
    sbrk(0);
    sbrk(4097);
    sbrk(0);
    sbrk(100000);
    sbrk(0);
    sbrk(-5);
    sbrk(0);

    int pid_2 = fork(); 
    if (pid_2 == 0) exit();
    sbrk(0);
    int value = 135;
  *((int*)a) = value;
  int* ptr = (int*)a;  
  int stored_value = *ptr;   
    printnum(stored_value);   
    write(1,"\n",1);
  }

    

  char spriteContent[] = {
    '*', '*', '*', ' o', 'o', 'o', 'o', 'o', 'o', 'o', 
    '*', '*', '*',  '*', 'o', 'o', 'o', 'o', 'o', 'o',
    'z', 'z', 'z', 'z', 'z', 'o', 'o', '*', 'X', '|',
    '*', '*', '*', '*', '*', '*', '*', '*', 'X', '|',
    '|', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', '|',
    '|', '-', '-', '-', '-', '-', '-', '-', '-', '|',
    '|', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', '|',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-'
  };

  Sprite sp = { 7, 10, spriteContent };

  int startCol = 40;
  int totalCols = 80;
  int totalRows = 25;

    for (int row = 0; row < totalRows; row++) {
        for (int col = startCol; col < totalCols; col++) {
            SetColor(col % 16, row % 16); 
            gotoXY(col, row);
        }
    }

  spritePut(50, 10, &sp);

  //threadCreate(111,0);

  while(1) {
    //if(getKey(&b) == 0) write(1,&b,1);
    
  }
}
