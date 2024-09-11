#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define _GNU_SOURCE
#include <poll.h>
#include <sys/syscall.h>
#include <sys/types.h>

static int pidfd_open(pid_t pid, unsigned int flags) {
   return syscall(SYS_pidfd_open, pid, flags);
}

int main (int argc, char *argv[]) {
	
	// prendo i parametri
  if(argc < 3) {
  	printf("troppi pochi arg");
  	exit(EXIT_FAILURE);
  }
  
  float duration = atof(argv[1]) / 1000.0;
  // crea un timer file descriptor basato sull'orologio di sistema
  int fdtimer = timerfd_create(CLOCK_REALTIME, 0);
  if(fdtimer == -1) {
    perror("timerfd_create");
    exit(EXIT_FAILURE);
  }
  
	int again;
  do {
  	again = 0;
		// struttura che contiene i dati del timer
		struct itimerspec newValue;
		// durata del timer
		newValue.it_value.tv_sec = duration;
		newValue.it_value.tv_nsec = 0;
		// ogni quanto ripeterlo
		newValue.it_interval.tv_sec = 0;
		newValue.it_interval.tv_nsec = 0;
		
		/* set events */
		struct pollfd exp;
		exp.fd = fdtimer;
		exp.revents = 0;
		exp.events = POLLIN;
		struct pollfd term;
		term.revents = 0;
		term.events = POLLHUP;
		
		
		// imposta il timer
		if(timerfd_settime(fdtimer, 0, &newValue, NULL) == -1) {
		  perror("timerfd_settime");
		  exit(EXIT_FAILURE);
		}
		
		int status = 0, ready;
		pid_t pid = fork();
		  
		if(pid == -1) {
		  perror("fork");
		  exit(EXIT_FAILURE);
		} else if(pid == 0) {
		  // processo figlio
		  char *arg[argc - 2];
		  for(int i = 3; i < argc; i++) {
		    arg[i - 3] = argv[i];
		  }
		  arg[argc - 3] = NULL;
		  
		  if(execve(argv[2], arg, NULL) == -1) {
		    perror("execve");
		    exit(EXIT_FAILURE);
		  }
		} else {
		  // processo padre
		  term.fd = pidfd_open(pid, 0);
		  struct pollfd pollevents[2] = {exp, term};
		 	
		 	while(1) {
		  	int ready = poll(pollevents, 2, -1);
		    if (ready == -1) {
		      perror("poll");
		      exit(EXIT_FAILURE);
		    }

		    if (pollevents[0].revents & POLLIN) {
		      printf("Timer expired\n");
		      kill(pid, SIGKILL);
		      break;
		    }

		    if (pollevents[1].revents & POLLHUP) {
		      waitpid(pid, &status, 0);
		      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		        printf("Child exited successfully\n");
		      } else {
		        printf("Child exited with errors\n");
		        again = 1;
		      }
		      break;
		    }
		  }
		}
  } while (again);
	
	return 0;
}
