#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#define L 100

int main(int argc, char *argv[]) {
  // creazione named pipe
  if (mkfifo(argv[1], 0777) == -1) {
    perror("mkfifo");
    return -1;
  }
  
  int fifo_fd = open(argv[1], O_RDONLY);
  if (fifo_fd == -1) {
    perror("open");
    return -1;
  }
  
  char buf[L] = {0};
  ssize_t bytesRead;
  int go = 1;

  // lettura dati
  while (go) {
    bytesRead = read(fifo_fd, buf, L);
    if(bytesRead <= 0) {
      close(fifo_fd);
      if((fifo_fd = open(argv[1], O_RDONLY)) == -1) {
        perror("open");
        return -1;
      }
    } else {
      if(buf[0] == 'F' && buf[1] == 'I' && buf[2] == 'N' && buf[3] == 'E') {
        close(fifo_fd);
        go = 0;
      } else {
        printf("%s\n", buf);
        pid_t pid = 0;
        int sig = 0, readingPid = 1, ok = 1;
        for (int i = 0; i < bytesRead; i++) {
          if(isdigit(buf[i]) && readingPid == 1) {
            pid = pid*10 + atoi(&buf[i]);
          } else if(isdigit(buf[i]) && readingPid == 0){
            sig = sig*10 + atoi(&buf[i]);
          } else if(buf[i] == ' '){
            readingPid = 0;
          } else {
            printf("Formatting error\n");
            ok = 0;
            break;
          }
        }
        if(ok == 1){
          // send signal
          if(kill(pid, sig) == -1) perror("kill");
        }
      }
    }
    memset(buf, 0, L);
  }

  return 0;
}
