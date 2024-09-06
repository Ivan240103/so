#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int min(int a, int b) {
  if (a < b) return a;
  else return b;
}

int main(int argc, char *argv[]) {
  int procNum = 2, expprnt = 3;
  if(getopt(argc, argv, "j:") == 'j') {
    procNum = atoi(optarg);
    expprnt += 2;
  }
  
  if(argc == expprnt) {
    // processo iniziale
    int srcfd = open(argv[argc - 2], O_RDONLY);
    if(srcfd == -1) return -1;
    int dstfd = open(argv[argc - 1], O_WRONLY | O_TRUNC | O_CREAT, 0755);
    if(dstfd == -1) {
      close(srcfd);
      return -1;
    }
    
    struct stat statbuf;
    fstat(srcfd, &statbuf);
    
    for(int i = 0; i < procNum; i++) {
      pid_t pid = fork();
      
      if(pid == -1) {
        perror("fork");
        return -1;
      } else if(pid == 0) {
        // processo figlio
        char arg1[10], arg2[10], arg3[25], arg4[12];
        sprintf(arg1, "%d", srcfd);
        sprintf(arg2, "%d", dstfd);
        sprintf(arg3, "%ld", statbuf.st_size / procNum);
        sprintf(arg4, "%d", i);
        char *arg[] = {arg1, arg2, arg3, arg4, NULL};
        // esecuzione del programma
        if(execv("./pcp", arg) == -1) {
          perror("execv");
          return -1;
        }
      } else {
        // processo padre
        if(i == procNum - 1) {
          if(waitpid(-1, NULL, __WALL) == -1) {
            perror("waitpid");
            return -1;
          }
          close(srcfd);
          close(dstfd);
        }
      }
    }
  } else if(argc == 4) {
    // figlio copiatore
    char buf[20] = {0};
    ssize_t count;
    off_t remain;
    int off;
    
    remain = atoi(argv[2]);
    off = remain * atoi(argv[3]);
    while(remain > 0) {
      count = pread(atoi(argv[0]), buf, min(sizeof(buf), remain), off);
      printf("Byte letti process %d: %ld\n", atoi(argv[3]), count);
      remain -= count;
      pwrite(atoi(argv[1]), buf, count, off);
      off += count;
    }
  } else {
    // errore
    printf("Bad usage. ARGC = %d\n", argc);
    return -1;
  }
  
  return 0;
}
