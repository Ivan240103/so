// Leggere dati da un file riga per riga e stamparli sul terminale

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define L 200

int main(int argc, char *argv[]) {
  char buf[L] = {0};
  
  // using syscall only
  int fd = open("./foo.txt", O_RDONLY);
  if(fd != -1) {
    // legge 200 caratteri dal file
    read(fd, buf, L);
    // prende la prima riga (token fino a \n)
    char *tok = strtok(buf, "\n");
    while(tok != NULL) {
      // finchè ci sono righe le stampa
      write(1, tok, strlen(tok));
      write(1, "\n", 1);
      // passa al token successivo (prossima riga)
      tok = strtok(NULL, "\n");
    }
    close(fd);
  } else {
    perror("open");
  }
  
  // using c function
  FILE *f = fopen("./foo.txt", "r");
  // legge una stringa
  while (fscanf(f, "%s", buf) != EOF) {
    // stampa la stringa letta
    printf("%s\n", buf);
  }
  fclose(f);
  
  return 0;
}
