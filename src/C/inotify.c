#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#define L 200

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("Bad usage\n");
    exit(EXIT_FAILURE);
  }
  
  char *buf[L] = {0};
  
  // create inotify file descriptor
  int inot_fd = inotify_init();
  if(inot_fd < 0) {
    perror("inotify_init");
  }
  // set up watcher
  int inot_w = inotify_add_watch(inot_fd, "./", IN_MOVED_TO);

  while(1) {
    if(read(inot_fd, buf, L) < 0) perror("read");
    
    struct inotify_event *event = (struct inotify_event*) buf;
    if (event->len) {
      if (event->mask & IN_MOVED_TO) {
        // the file event->name is moved
        char path[L] = {};
        strcat(path, "./");
        strcat(path, event->name);
        printf("Path del file mosso: %s\n", path);
      }
    }
  }
  
  // remove watcher
  inotify_rm_watch(inot_fd, inot_w);
  close(inot_fd);

  return 0;
}
