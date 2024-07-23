#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  struct stat statbuf;
  char *dirName = "...";
  
  // creazione cartella se non esiste
  if(stat(dirName, &statbuf) < 0) {
    if(mkdir(dirName, 0755) < 0) {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }
  }
  
  // leggo il contenuto della dir corrente
  DIR *dir;
  struct dirent *entry;
  char path[PATH_MAX] = {0};
  
  getcwd(path, PATH_MAX);
  
  // apro la directory
  if((dir = opendir(path)) == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
  }
  
  while((entry = readdir(dir)) != NULL) {
    // prendo info
    if(lstat(entry->d_name, &statbuf) == -1) {
      perror("lstat");
      continue;
    }
    
    if(S_ISREG(statbuf.st_mode) && strcmp(entry->d_name, "p") != 0) {
      snprintf(path, PATH_MAX, "./%s/%s", dirName, entry->d_name);
      
      // copio il file nella subdir
      char buffer[1024] = {0};
      int files[2];
      ssize_t count;
      
      files[0] = open(entry->d_name, O_RDONLY);
      if (files[0] == -1)
        return -1;
      files[1] = open(path, O_WRONLY | O_CREAT, 0755);
      if (files[1] == -1) {
        close(files[0]);
        return -1;
      }

      while ((count = read(files[0], buffer, sizeof(buffer))) != 0)
        write(files[1], buffer, count);

      //crea link
      if(symlink(path, "zzz") < 0) {
      	perror("symlink");
      }
      //rinomina link
      if(rename("zzz", entry->d_name) < 0) {
      	perror("rename");
      }
    }
  }
  
  return 0;
}
