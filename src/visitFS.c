// Visita dell'albero del file system a partire da una directory
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void visit(char* dirName);

int main(int argc, char *argv[]) {
  char *start = ".";
  if(argc > 1) start = argv[1];
  visit(start);
  return 0;
}

void visit(char* dirName) {
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;
  char path[PATH_MAX];
  
  // apro la directory
  if((dir = opendir(dirName)) == NULL) {
    perror("opendir error");
    return;
  }
  
  // cambio la directory corrente
  if(chdir(dirName) == -1) {
    perror("chdir error");
    closedir(dir);
    return;
  }
  
  while((entry = readdir(dir)) != NULL) {
    // ignoro . e .. per evitare loop
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    
    // prendo info sula directory
    if(lstat(entry->d_name, &statbuf) == -1) {
      perror("lstat error");
      continue;
    }
    
    // FACCIO COSE CON LA DIRECTORY...
    
    // se è una directory la visito ricorsivamente
    if(S_ISDIR(statbuf.st_mode)) {
      printf("Directory: %s\n", entry->d_name);
      snprintf(path, sizeof(path), "%s/%s", dirName, entry->d_name);
      visit(path);
    } else {
      printf("File: %s\n", entry->d_name);
    }
  }
  
  // torno alla directory precedente
  if(chdir("..") == -1) {
    perror("chdir finale error");
  }
  
  closedir(dir);
}
