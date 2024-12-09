#include <libc.h>

char buff[24];

int pid;
int getKey(char* b);
char* sbrk(int size);
int SetColor(int color, int background);
int gotoXY(int posX, int posY);
int spritePut(int posX, int posY, Sprite* sp);

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
  char b;
  char* a;
  char* c;
  int bytes  = 0;
  a = sbrk(1); 
  bytes = write(1,"El char en a vale: ",18);
  if(bytes < 0) perror();
  *a = 'o'; // este char se copiará al hijo
  bytes = write(1, a, 1);
  if(bytes < 0) perror();
  write(1,"\n",1);

  write(1,"Comprobar acces_ok\n",19);
  //si accedes fuera del heap, da error 
  bytes = write(1,"El char en a vale: ",18);
  if(bytes < 0) perror();
  bytes = write(1, a+100, 1);
  if(bytes < 0) perror(); // se escribe el perror
  write(1,"\n",1);

  

  c = sbrk(4); 
  int value = 42;
  *((int*)c) = value;
  int* ptr = (int*)c;  
  int stored_value = *ptr;
  printnum(stored_value);   
  write(1,"\n",1); 
  
  //sbrk(100000000); // comprobar fin de paginas

  sbrk(-1000000); // comprobar que no puedes pasar el limite inferior

  pid = fork();
  if(pid == 0) {
    write(1,"El hijo ha empezado\n",20);
    sbrk(0);
    bytes = write(1, a, 1); // comprobar que el valor de a en el heap se ha copiado  
    if(bytes < 0) perror();
    write(1,"\n",1);
    int* ptr = (int*)c;   
    int stored_value = *ptr;
    printnum(stored_value); // comprobar que el valor de c en el heap se ha copiado   
    write(1,"\n",1);
    sbrk(-5);
    sbrk(0);
    sbrk(-2); // comprobar que no puedes pasar el limite inferior
    exit();
  } else {
    sbrk(0);
    *a = 'x';
    bytes = write(1, a, 1); // comprobar que para el padre el valor de a despues del fork puede ser otro y no afecta al heap del hijo  
    if(bytes < 0) perror();
    write(1,"\n",1);
    write(1,"El padre ha acabado\n",20);
    
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

  while(1) { 
    //if(getKey(&b) == 0) write(1,&b,1);
  }
}
