// Timer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char* argv[]) {
  // prendo i parametri
  if(argc < 1) exit(EXIT_FAILURE);
  int rep = atoi(strtok(argv[1], ","));
  float interv = atof(strtok(NULL, ","));
  char* str = strtok(NULL, ",");
  
  // crea un timer file descriptor basato sull'orologio di sistema
  int timer = timerfd_create(CLOCK_REALTIME, 0);
  if(timer == -1) {
    perror("create timer error");
    exit(EXIT_FAILURE);
  }
  
  // struttura che contiene i dati del timer
  struct itimerspec newValue;
  // durata del timer
  newValue.it_value.tv_sec = 0;
  newValue.it_value.tv_nsec = 0;
  // ogni quanto ripeterlo
  newValue.it_interval.tv_sec = interv;
  newValue.it_interval.tv_nsec = 0;
  
  // imposta i valori del timer
  if(timerfd_settime(timer, 0, &newValue, NULL) == -1) {
    perror("set timer error");
    exit(EXIT_FAILURE);
  }
  
  // ripeti il timer un numero rep di volte
  while(rep > 0) {
    // legge il contenuto del file del timer
    // lettura bloccante per leggere solo quando scatta il timer
    uint64_t exp;
    ssize_t s = read(timer, &exp, sizeof(uint64_t));
    // FACCIO COSE DOPO CHE IL TIMER È SCATTATO
    printf("%s\n", str);
    
    rep--;
  }
  
  return 0;
}
