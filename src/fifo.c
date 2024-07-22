// named pipe (FIFO)

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define L 200

// READER
int main(int argc, char *argv[]) {
  // creazione named pipe
  if (mkfifo("/tmp/my_fifo", 0766) == -1) {
    perror("mkfifo");
    exit(EXIT_FAILURE);
  }
  
  int fifo_fd = open("/tmp/my_fifo", O_RDONLY);
  if (fifo_fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  
  char buf[L] = {0};
  ssize_t bytesRead;

  // lettura dati
  while (1) {
    bytesRead = read(fifo_fd, buf, L);
    if(bytesRead < 0) {
      perror("read");
      close(fifo_fd);
      exit(EXIT_FAILURE);
    } else if(bytesRead == 0) {
      close(fifo_fd);
      fifo_fd = open("/tmp/my_fifo", O_RDONLY);
      if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
      }
    } else {
      write(STDOUT_FILENO, buf, strlen(buf));    
    }
    memset(buf, 0, L);
  }
  
  close(fifo_fd);

  return 0;
}

// WRITER
/* int fifo_fd = open("/tmp/my_fifo", O_WRONLY);
if (fifo_fd == -1) {
  perror("open");
  exit(EXIT_FAILURE);
}

char *msg = "Hello world!\n";
write(fifo_fd, msg, strlen(msg));

close(fd); */
