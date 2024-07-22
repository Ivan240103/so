#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define L 200

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("Bad usage\n");
    exit(EXIT_FAILURE);
  }
  
  char buf[L] = {0};
  
  // create inotify file descriptor
  int inot_fd = inotify_init();
  if(inot_fd < 0) {
    perror("inotify_init");
  }
  // set up watcher
  int inot_w = inotify_add_watch(inot_fd, argv[1], IN_MOVED_TO);

  while(1) {
    if(read(inot_fd, buf, L) < 0) perror("read");
    
    struct inotify_event *event = (struct inotify_event*) buf;
    if (event->len) {
      if (event->mask & IN_MOVED_TO) {
        // the file event->name is moved
        char path[L] = {};
        strcat(path, argv[1]);
        strcat(path, event->name);
        
        int file_fd = open(path, O_RDONLY);
        if(file_fd != -1) {
          char param[L] = {0};
          read(file_fd, param, L);
          char *arg[10];
          int i = 0;
          char *eseg = strtok(param, "\n");
          char *tok = strtok(NULL, "\n");
          while(tok != NULL) {
            arg[i++] = tok;
            tok = strtok(NULL, "\n");
          }
          arg[i] = NULL;
          close(file_fd);
          unlink(path);
          
          int status;
          pid_t pid = fork();
          
          if(pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
          } else if(pid == 0) {
            // proc figlio
            if(execve(eseg, arg, NULL) == -1) {
              perror("execve");
              exit(EXIT_FAILURE);
            }
          } else {
            // proc padre
            waitpid(pid, &status, 0);
          }
        } else {
          perror("open");
        }
      }
    }
  }
  
  // remove watcher
  inotify_rm_watch(inot_fd, inot_w);
  close(inot_fd);

  return 0;
}
