#include <libc.h>
#include <stddef.h>

char buff[24];

int pid;
int sem_id;
int game_running;
int getKey(char* b);
char* sbrk(int size);
int SetColor(int color, int background);
int gotoXY(int posX, int posY);
int spritePut(int posX, int posY, Sprite* sp);
int threadCreate(void(*function)(void* arg), void* parameter);
int semCreate(int initial_value);
int semWait(int semid);
int semSignal(int semid);
int semDestroy(int semid);
void clean_screen();
void clean_region(int startX, int startY, int endX, int endY);
void threadExit();
unsigned int gettime();


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

void test (void* n) {
  int i = 0;
  int p = fork();
  printnum(p);
  while (p == 0) write (1, "hijo", 4);
  while (1) {
    if (write (1, (char *)n, 1) != -1);
    ++i;
  }
}

void test2 (void* n) {
  semWait(*(int *)n);
  int i = 0;
  while (i < 10000) {
    write (1, "b", 1);
    ++i;
  }
  semSignal(1);
}

void wrapper_func(void * (*func) (void *param), void * param) {
	if(func != NULL && param != NULL) {
		func(param);
		threadExit();
	}
}

typedef struct {
    int x, y;
    Sprite b;
} Barras;

typedef struct {
    int x, y; 
    int dx, dy;
    Sprite b; 
} Bola;

typedef struct {
    Barras br;
    Barras bl;
    Bola ball;
    int left_score;
    int right_score;
} InfoGame;

InfoGame game_state;

void init_game() {
    char* barra_ascii = (char*)sbrk(8 * sizeof(char));
    barra_ascii[0] = '|'; barra_ascii[1] = '|'; barra_ascii[2] = '|'; barra_ascii[3] = '|';
    barra_ascii[4] = '|'; barra_ascii[5] = '|'; barra_ascii[6] = '|'; barra_ascii[7] = '|'; 
    Sprite sp = {1, 8, barra_ascii};
    game_state.bl.b = sp;
    game_state.bl.x = 2;
    game_state.bl.y = (25 / 2) - 3;

    game_state.br.b = sp;
    game_state.br.x = 80 - 3;
    game_state.br.y = (25 / 2) - 3;

    char* bola_ascii = (char*)sbrk(1 * sizeof(char));
    bola_ascii[0] = 'x';
    Sprite spa = {1, 1, bola_ascii};
    game_state.ball.x = 80 / 2 - 3;
    game_state.ball.y = 25 / 2;
    game_state.ball.dx = 1;
    game_state.ball.dy = 1;
    game_state.ball.b = spa;

    game_state.left_score = 0;
    game_state.right_score = 0;
}

void draw_game() {
    semWait(sem_id);

    static int prev_left_score = -1, prev_right_score = -1;
    if (game_state.left_score != prev_left_score || game_state.right_score != prev_right_score) {
        gotoXY(25, 2);
        write(1, "Puntuacion: ", 12);
        char buffer[12];
        itoa(game_state.left_score, buffer);
        write(1, buffer, strlen(buffer));
        write(1, " - ", 3);
        itoa(game_state.right_score, buffer);
        write(1, buffer, strlen(buffer));
        prev_left_score = game_state.left_score;
        prev_right_score = game_state.right_score;
    }

    semSignal(sem_id);
}

void input_thread(int * param) {
    char input;

    while (game_running) {
        if (getKey(&input) == 0) {
            clean_region(game_state.bl.x, 0, game_state.bl.x, 24);
            clean_region(game_state.br.x, 0, game_state.br.x, 24);

            if (input == 'w' && game_state.bl.y > 1) {
                game_state.bl.y -= 2;
            } else if (input == 's' && game_state.bl.y + game_state.bl.b.y < 24) {
                game_state.bl.y += 2;
            }

            if (input == 'i' && game_state.br.y > 1) {
                game_state.br.y -= 2;
            } else if (input == 'k' && game_state.br.y + game_state.br.b.y < 24) {
                game_state.br.y += 2;
            }

            spritePut(game_state.bl.x, game_state.bl.y, &game_state.bl.b);
            spritePut(game_state.br.x, game_state.br.y, &game_state.br.b);
        }
        for (int i = 0; i < 1000; i++);
    }
}

void reset_ball() {
    game_state.ball.x = 80 / 2;
    game_state.ball.y = 25 / 2;
    game_state.ball.dx = (game_state.ball.dx > 0) ? 1 : -1;
    game_state.ball.dy = (game_state.ball.dy > 0) ? 1 : -1;
}

void ball_thread(int *param) {
    unsigned int last_tick = gettime();
    int prev_x = game_state.ball.x;
    int prev_y = game_state.ball.y;

    while (game_running) {
        if (gettime() - last_tick >= 50) {
            last_tick = gettime();

            semWait(sem_id);

            clean_region(prev_x, prev_y, prev_x, prev_y);

            prev_x = game_state.ball.x;
            prev_y = game_state.ball.y;

            game_state.ball.x += game_state.ball.dx;
            game_state.ball.y += game_state.ball.dy;

            if (game_state.ball.y <= 1) {
                game_state.ball.dy = 1;
            } else if (game_state.ball.y >= 24) {
                game_state.ball.dy = -1;
            }

            if (game_state.ball.x == game_state.bl.x + 1 &&
                game_state.ball.y >= game_state.bl.y &&
                game_state.ball.y < game_state.bl.y + game_state.bl.b.y) {
                game_state.ball.dx = 1;
            }

            if (game_state.ball.x == game_state.br.x - 1 &&
                game_state.ball.y >= game_state.br.y &&
                game_state.ball.y < game_state.br.y + game_state.br.b.y) {
                game_state.ball.dx = -1;
            }

            if (game_state.ball.x <= 0) {
                game_state.right_score++;
                if (game_state.right_score == 3) {
                    gotoXY(25, 12);
                    write(1, "Jugador derecha gana!", 22);
                    game_running = 0;
                } else {
                    reset_ball();
                }
            } else if (game_state.ball.x >= 80) {
                game_state.left_score++;
                if (game_state.left_score == 3) {
                    gotoXY(25, 12);
                    write(1, "Jugador izquierda gana!", 24);
                    game_running = 0;
                } else {
                    reset_ball();
                }
            }

            spritePut(game_state.ball.x, game_state.ball.y, &game_state.ball.b);

            semSignal(sem_id);
        }
    }
}

int b = 18; // no sirve para nada

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  // char b;
  // char* a;
  // char* c;
  // int bytes  = 0;
  // a = sbrk(1); 
  // bytes = write(1,"El char en a vale: ",18);
  // if(bytes < 0) perror();
  // *a = 'o'; // este char se copiará al hijo
  // bytes = write(1, a, 1);
  // if(bytes < 0) perror();
  // write(1,"\n",1);

  // write(1,"Comprobar acces_ok\n",19);
  // si accedes fuera del heap, da error 
  // bytes = write(1,"El char en a vale: ",18);
  // if(bytes < 0) perror();
  // bytes = write(1, a+100, 1);
  // if(bytes < 0) perror(); // se escribe el perror
  // write(1,"\n",1);

  

  // c = sbrk(4); 
  // int value = 42;
  // *((int*)c) = value;
  // int* ptr = (int*)c;  
  // int stored_value = *ptr;
  // printnum(stored_value);   
  // write(1,"\n",1); 
  
  // sbrk(100000000); // comprobar fin de paginas

  // sbrk(-1000000); // comprobar que no puedes pasar el limite inferior

  // pid = fork();
  // if(pid == 0) {
  //   SetColor(5,0);
  //   write(1,"El hijo ha empezado\n",20);
  //   sbrk(0);
  //   bytes = write(1, a, 1); // comprobar que el valor de a en el heap se ha copiado  
  //   if(bytes < 0) perror();
  //   write(1,"\n",1);
  //   int* ptr = (int*)c;   
  //   int stored_value = *ptr;
  //   printnum(stored_value); // comprobar que el valor de c en el heap se ha copiado   
  //   write(1,"\n",1);
  //   sbrk(-5);
  //   sbrk(0);
  //   sbrk(-2); // comprobar que no puedes pasar el limite inferior
  //   exit();
  // } else {
  //   sbrk(0);
  //   *a = 'x';
  //   bytes = write(1, a, 1); // comprobar que para el padre el valor de a despues del fork puede ser otro y no afecta al heap del hijo  
  //   if(bytes < 0) perror();
  //   write(1,"\n",1);
  //   write(1,"El padre ha acabado\n",20);
    
  // }


  // char spriteContent[] = {
  //   ':','D',
  // };

  // Sprite sp = { 1, 2, spriteContent };

  // SetColor(2,6);
  // spritePut(78, 24, &sp);

  //  char a1 = 'a';
  //  char b2 = 'b';
  //  char c3 = 'c';
  //  char d4 = 'd';

  //probar la creacion de dibersos threads

  //  threadCreate(&test, &a1);
  //  threadCreate(&test, &b2);   
  //  threadCreate(&test, &c3);
  //  threadCreate(&test, &d4);
  
  //  int s1 = semCreate(0);
  //  int s2 = semCreate(0);
  

  //  threadCreate(&test2, &s1);

  // //  exit();
  //  int i = 0;
  //  while (i < 10000) {
  //    write (1, "a", 1);
  //    ++i;
  //  }
  //  semSignal(s1);  //desbloquear al thread 

  //  semWait(s2); //esperas al thread 
  //  i = 0;
  //  while (i < 10000) {
  //    write (1, "c", 1);
  //    ++i;
  //  }

  // semDestroy(s1); //eliminación de semaforos
  // semDestroy(s2); //eliminación de semaforos

  clean_screen();
    gotoXY(30, 12);
    write(1, "PULSA P PARA JUGAR", 18);
    gotoXY(3, 19);
    SetColor(6, 0);
    write(1, "CONTROLES IZQ: ", 15);
    SetColor(2, 0);
    write(1, "W arriba  S abajo ", 18);
    gotoXY(45, 19);
    SetColor(5, 0);
    write(1, "CONTROLES DER: ", 15);
    SetColor(2, 0);
    write(1, "I arriba  K abajo ", 18);

    char a;
    while (1) {
        if (getKey(&a) == 0) {
            if (a == 'p') {
                clean_screen();
                init_game();
                draw_game();

                sem_id = semCreate(1);
                game_running = 1;

                threadCreate(&ball_thread, &b);
                threadCreate(&input_thread, &b);

                while (game_running) {
                    draw_game();
                }

                semDestroy(sem_id);

                clean_screen();
                gotoXY(25, 12);
                write(1, "El juego ha terminado", 21);
				gotoXY(25, 14);
                write(1, "Pulsa P para reiniciar", 22);
            }
        }
    }
}
