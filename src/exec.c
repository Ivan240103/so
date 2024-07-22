// Esecuzione di un programma tramite un nuovo processo

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  int status;
  // creazione figlio
  pid_t pid = fork();
  
  if(pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if(pid == 0) {
    // sta eseguendo il processo figlio
    printf("Processo figlio (PID: %d) del padre (PID: %d)\n", getpid(), getppid());
    // prepara gli argomenti per execve
    char *arg[] = {"ls", "-l"};
    // esecuzione del programma
    if(execve("/bin/ls", arg, NULL) == -1) {
      perror("execve");
      exit(EXIT_FAILURE);
    }
  } else {
    // sta eseguendo il processo padre
    printf("Processo padre (PID: %d) del figlio (PID: %d)\n", getpid(), pid);
    // aspetta che il figlio termini
    if(waitpid(pid, &status, 0) == -1) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    
    // stato di uscita del figlio
    if(WIFEXITED(status)) {
      printf("Figlio è terminato con stato %d\n", WEXITSTATUS(status));
    } else {
      printf("Figlio non terminato correttamente\n");
    }
  }
  
  return 0;
}
