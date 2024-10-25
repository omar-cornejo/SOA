/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

int zeos_ticks = 0;

char char_map[] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '\'', 'i', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '`', '+', '\0', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ñ',
    '\0', 'º', '\0', 'ç', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '-', '\0', '*',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '7',
    '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '\0', '\0', '\0', '<', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void keyboard_handler();
void pf_handler();
void clock_handler();
void write_msr(unsigned long register, unsigned long address);
void syscall_handler_sysenter();


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(33, keyboard_handler, 0 );
  setInterruptHandler(14, pf_handler, 0);
  setInterruptHandler(32, clock_handler, 0);

  //crear funcion
  write_msr(0x174,__KERNEL_CS);
  write_msr(0x175,INITIAL_ESP);
  write_msr(0x176,(unsigned long)syscall_handler_sysenter);
  
  set_idt_reg(&idtR);
}


void keyboard_routine(){
  // Read from port 0x60 (pv = port value)
  unsigned char pv = inb(0x60);
  // 0x80 = 10000000
  // 0x80 & 00000000 = 0 // Make
  // 0x80 & 10000000 = 1 // Break
  int isbreak = pv & 0x80;

  if(isbreak == 0){ // If it is a make
    char toprint = char_map[pv & 0x7F];
    printc_xy(70, 20, toprint);
  }
}

char hex[9];

void unsignedIntToHex(unsigned int num) {
        // Un entero de 32 bits puede ser representado por 8 dígitos hexadecimales + 1 para el terminador nulo.
        int i = 7;    // Posición del último dígito hexadecimal

        // Inicializamos la cadena con ceros
        for (int j = 0; j < 8; j++) {
            hex[j] = '0';
        }
        hex[8] = '\0';  // Terminador de la cadena

        // Convertir a hexadecimal
        while (num != 0) {
            int remainder = num % 16;
            if (remainder < 10) {
                hex[i] = remainder + '0';  // Si es de 0-9, convertimos a carácter '0'-'9'
            } else {
                hex[i] = remainder - 10 + 'A';  // Si es de 10-15, convertimos a 'A'-'F'
            }
            num = num / 16;
            i--;
        }
      printk(hex);
}

void pf_routine(unsigned int error,unsigned int eip){

  printk("Erro de codigo:");
  unsignedIntToHex(error);
  printk("\n");
  printk("Erro de instruccion:");
  unsignedIntToHex(eip);
  printk("\n");
  while(1);  
}


void clock_routine() {
  zeos_ticks++;
  if (zeos_ticks > 1000 )
  {
    task_switch((union task_union*) &task[1]);
  }
  
  zeos_show_clock();
}
