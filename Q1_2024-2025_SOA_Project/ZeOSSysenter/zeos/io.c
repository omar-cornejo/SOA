/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printnum(int num)
{
  char buffer[16]; // Suponiendo que un número entero no excede 16 dígitos
  int i = 0;
  
  // Convierte el número a una cadena hexadecimal (puedes cambiar el formato si prefieres otro tipo)
  if (num == 0)
  {
    buffer[i++] = '0';
  }
  else
  {
    while (num > 0)
    {
      int digit = num % 16;
      buffer[i++] = (digit < 10) ? ('0' + digit) : ('a' + (digit - 10));
      num /= 16;
    }
  }
  
  // Imprime la cadena de números en orden correcto
  for (int j = i - 1; j >= 0; j--)
  {
    printc(buffer[j]);
  }
}


void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}
