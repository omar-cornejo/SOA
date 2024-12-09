/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/** Screen functions **/
/**********************/

typedef struct {
    int x;
    //number of rows
    int y;
    //number of columns
    char* content; //pointer to sprite content matrix(X,Y)
} Sprite;

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);
void printnum(int num);

#endif  /* __IO_H__ */
