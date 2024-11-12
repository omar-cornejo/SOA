#include <libc.h>

char buff[4];

int pid;

int addASM(int, int);

int zeos_ticks;
int write(int fd, char* buffer,int size);
int gettime();
void exit();
void block();
int unblock(int pid);

int __attribute__ ((__section__(".text.main")))
  main(void)
{

	// int numero = addASM(0x42,0x666);
    
  //int* numero = 0;
  //*numero = 0;


  int bytes;

  // bytes = write(1, "Ejemplo write\n",14);
  // if(bytes == -1) perror();

  // bytes = write(1, "Ahora muestro mi pid: ",22);
  // if(bytes == -1) perror();

  // int pid = getpid();
  // itoa(pid,buff);
  // bytes = write(1,buff,strlen(buff));
  // if(bytes == -1) perror();
  // bytes = write(1,"\n",1);
  // if(bytes == -1) perror();


  int pid_fork = fork();
  itoa(pid_fork,buff);
  if(pid_fork == 0) {
    bytes = write(1,"Soy el hijo con pid: ",21);
    if(bytes == -1) perror();
    itoa(getpid(), buff);
    bytes = write(1,buff,strlen(buff));
    if(bytes == -1) perror();
    bytes = write(1,"\n",1);
    write(1,"Me voy a bloquear\n",18);
    block();
    write(1,"Me han desbloqueado\n",20);
  }
  else {
    bytes = write(1, "Soy el padre con pid:",21);
    if(bytes == -1) perror();
    itoa(getpid(),buff);
    bytes = write(1,buff,strlen(buff));
    if(bytes == -1) perror();
    bytes = write(1,"\n",1);
    //en este block el hijo todavia no está bloqueado, se decremente a -1
    bytes = unblock(pid_fork);
  }
  
  //si comentas se desbloquea el hijo
  if(pid_fork != 0)unblock(pid_fork);

  //no se encuentra el hijo
  if(pid_fork != 0) {
    unblock(7);
    exit();
  }
  else {
    exit();
  }

  // crear más hijos
  // int pid_fork2;
  // pid_fork = fork();
  // if(pid_fork != 0) {
  //     pid_fork2 = fork();
  //     if(pid_fork2 == 0 ){
  //       unblock(2);
  //       //descomentar para matar al hijo
  //       //exit();
  //     }
  // }
  // else {
  //   unblock(2);
  //       //descomentar para matar al hijo
  //       //exit();
  // }

  // if(pid_fork != 0 && pid_fork2 != 0) {
  //   unblock(pid_fork);
  //   unblock(pid_fork2);
  // }

  while(1) {
    
    //comprobar los procesos que quedan
    // int a = getpid();  
    // char *puntero = buff;
    // itoa(a,puntero);
    // if((write(1,"Proeceso con pid:",17)) == -1) perror();
    // if((write(1,buff,strlen(buff))) == -1) perror();
    // if((write(1,"\n",1)) == -1) perror();
    
  }
} 
