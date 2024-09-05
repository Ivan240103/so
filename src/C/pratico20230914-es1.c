#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

int countFile(const char *path);
char **vreaddir(const char *path);

int main(int argc, char *argv[]) {
  char *path = "/home/ivan/Pictures/Screenshots/";
  char **strings = vreaddir(path);
  int k = 0;
  while(strings[k] != NULL) {
    printf("%s\n", strings[k]);
    k++;
  }

  // Free memory for each string
  for (int i = 0; i <= k; i++) {
    free(strings[i]);
  }
  // Free memory for the array of pointers
  free(strings);
  return 0;
}

char **vreaddir(const char *path) {
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;
  int size = countFile(path) + 1;
  char** names = NULL;
  // alloco memoria per l'array di stringhe
  names = (char**)malloc(size * sizeof(char*));
  
  // apro la directory
  if((dir = opendir(path)) == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
  }
  
  // cambio la directory corrente
  if(chdir(path) == -1) {
    perror("chdir");
    closedir(dir);
    exit(EXIT_FAILURE);
  }
  // contatore
  int i = 0;
  while((entry = readdir(dir)) != NULL) {
    // prendo info
    if(lstat(entry->d_name, &statbuf) == -1) {
      perror("lstat");
      exit(EXIT_FAILURE);
    }
    
    // se è un file ne prendo il nome
    if(S_ISREG(statbuf.st_mode)) {
      names[i] = (char*)malloc((strlen(entry->d_name) + 1) * sizeof(char));
      sprintf(names[i], "%s", entry->d_name);
      i++;
    }
  }
  names[i] = NULL;
  
  closedir(dir);
  return names;
}

int countFile(const char *path) {
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;
  int n = 0;
  
  // apro la directory
  if((dir = opendir(path)) == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
  }
  
  // cambio la directory corrente
  if(chdir(path) == -1) {
    perror("chdir");
    closedir(dir);
    exit(EXIT_FAILURE);
  }
  
  while((entry = readdir(dir)) != NULL) {
    // prendo info
    if(lstat(entry->d_name, &statbuf) == -1) {
      perror("lstat");
      exit(EXIT_FAILURE);
    }
    
    // controllo se è un file
    if(S_ISREG(statbuf.st_mode)) n++;
  }
  
  closedir(dir);
  return n;
}
