#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
  int status = 0;
  struct timeval start, end;
  
  do {
    if(gettimeofday(&start, NULL) == -1) perror("gettimeofday");
    
    pid_t pid = fork();
    
    if(pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    } else if(pid == 0) {
      // processo figlio
      char *arg[argc - 1];
      for(int i = 2; i < argc; i++) {
        arg[i - 2] = argv[i];
      }
      arg[argc - 2] = NULL;
      
      if(execve(argv[1], arg, NULL) == -1) {
        perror("execve");
        exit(EXIT_FAILURE);
      }
    } else {
      // processo padre
      if(waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      
      // stato di uscita del figlio
      if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Figlio terminato con successo\n");
        if(gettimeofday(&end, NULL) == -1) perror("gettimeofday");
        printf("tempo di esecuzione: %ld us\n", (end.tv_usec - start.tv_usec));
      } else {
        printf("Figlio terminato con errori\n");
      }
    }
    sleep(3);
  } while(!status && (end.tv_usec - start.tv_usec) < 1000000);
  
  return 0;
}
