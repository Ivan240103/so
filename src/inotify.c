#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#define LEN 255

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("bad usage\n");
    return -1;
  }
  
  char buffer[LEN];

  int fd = inotify_init();
  if(fd < 0) {
    perror("inotify_init");
  }

  int wd = inotify_add_watch(fd, argv[1], IN_MOVED_TO);

  while (1) {
    if(read(fd, buffer, LEN) < 0) perror("read");
    
    struct inotify_event *event = (struct inotify_event *) &buffer;
      if (event->len) {
        if (event->mask & IN_MOVED_TO) {
          // the file event->name is moved
          char path[LEN] = {};
          strcat(path, argv[1]);
          strcat(path, event->name);
          FILE *f = fopen(path, "r");
          char* cont;
          size_t len = 0;
          while (getline(&cont, &len, f) != -1) {
            printf("%s", cont);
          }
          fclose(f);
        }
      }
  }

  inotify_rm_watch(fd, wd);
  close(fd);

  return 0;
}
