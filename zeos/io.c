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

void scrollScreen() {
    Word *screen = (Word *)0xb8000;

    // Desplazar todas las filas una posición hacia arriba
    for (int row = 1; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLUMNS; col++) {
            screen[(row - 1) * NUM_COLUMNS + col] = screen[row * NUM_COLUMNS + col];
        }
    }

    // Limpiar la última fila (llenarla con espacios)
    for (int col = 0; col < NUM_COLUMNS; col++) {
        screen[(NUM_ROWS - 1) * NUM_COLUMNS + col] = 0x0720; // Espacio con fondo negro y texto gris
    }
}


void printc(char c)
{
    __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */

    if (c == '\n') {
        x = 0;
        y++;
    }
    else {
        Word ch = (Word) (c & 0x00FF) | 0x0200; // Carácter con atributos de color
        Word *screen = (Word *)0xb8000;
        screen[(y * NUM_COLUMNS + x)] = ch;

        if (++x >= NUM_COLUMNS) { // Si llegamos al final de la fila
            x = 0;
            y++;
        }
    }

    // Si llegamos al final de la pantalla, hacer scroll
    if (y >= NUM_ROWS) {
        scrollScreen();
        y = NUM_ROWS - 1; // Ajustar el cursor a la última fila después del scroll
    }
}


void printc_color(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0600;
  Word *screen = (Word *)0xb8000;
  screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
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

void printk_color(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc_color(string[i]);
}


void print_number(int num)
{
  char str[12];
  int i = 0;
  if (num < 0)
  {
    printc('-');
    num = -num;
  }
  if (num == 0)
  {
    printc('0');
    return;
  }
  while (num > 0)
  {
    str[i++] = num % 10 + '0';
    num /= 10;
  }
  str[i] = 0;
  i--;
  while (i >= 0)
    printc(str[i--]);
}


