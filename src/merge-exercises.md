---
tags:
  - sistemi-operativi
  - C
  - python
  - linux
Creation: 20_06_2024-11:45
lucidi:
  - "[[2023.02.16.pdf]]"
---
# Prova 2023-09-07
#exec #time 

## Func
- `clock`

- `file.read`
- ELF->`b'\0x7fELF`
## esercizio es2
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAXTIME 1

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s command [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // check the time that the command takes to execute
    double time_spent;
    clock_t begin, end;
    int status;
    do {
        begin = clock();

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (pid == 0)
            execvp(argv[1], &argv[1]);

        wait(&status);
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Command %s took %f seconds to execute\n", argv[1], time_spent);
    } while (!status && time_spent > MAXTIME);

    return EXIT_SUCCESS;
}
```

## esrcizio es3
```python
#!/usr/bin/python3

import os
import sys

def is_elf(file_path):
  try:
    with open(file_path, 'rb') as file:
      header = file.read(4)
      return header == b'\x7fELF'
  except:
    return False

paths = sys.argv[1:] if len(sys.argv) >= 2 else ['.']
for path in paths:
  for root, _, files in os.walk(path):
    for file in files:
      if is_elf(os.path.join(root, file)):
        print(os.path.join(root, file))

```
# Prova 2023-01-19
#pipe #commands #multipipe 
## Useful functions
- `popen` -> pipe a command
- `fgets/fputs` -> not recommended
## esercizio es1
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 1024

void execute_commands(const char *command1, const char *command2) {
    FILE *pipe1;
    FILE *pipe2;
    char buf[MAX_COMMAND_LENGTH];

    // Apre il primo comando in lettura
    if ((pipe1 = popen(command1, "r")) == NULL) {
        perror("Errore nell'apertura del primo comando");
        exit(EXIT_FAILURE);
    }

    // Apre il secondo comando in scrittura
    if ((pipe2 = popen(command2, "w")) == NULL) {
        perror("Errore nell'apertura del secondo comando");
        exit(EXIT_FAILURE);
    }

    // Legge l'output del primo comando e lo scrive nel secondo
    while (fgets(buf, MAX_COMMAND_LENGTH, pipe1) != NULL) {
        fputs(buf, pipe2);
    }

    // Chiude i pipe
    pclose(pipe1);
    pclose(pipe2);
}

int main() {
    char command1[MAX_COMMAND_LENGTH];
    char command2[MAX_COMMAND_LENGTH];

    // Legge i due comandi da stdin
    if (fgets(command1, MAX_COMMAND_LENGTH, stdin) == NULL) {
        perror("Errore nella lettura del primo comando");
        exit(EXIT_FAILURE);
    }
    // Rimuove il newline finale se presente
    command1[strcspn(command1, "\n")] = '\0';

    if (fgets(command2, MAX_COMMAND_LENGTH, stdin) == NULL) {
        perror("Errore nella lettura del secondo comando");
        exit(EXIT_FAILURE);
    }
    // Rimuove il newline finale se presente
    command2[strcspn(command2, "\n")] = '\0';

    // Esegue i due comandi in sequenza
    execute_commands(command1, command2);

    return 0;
}
```
## esercizio es2
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_COMMANDS 10
#define MAX_BUFFERS 10

void execute_commands(int num_commands, char *commands[], int num_buffers, char *buffers[]) {
    FILE *pipes[MAX_COMMANDS];
    int pipefd[2 * (num_commands - 1)];
    pid_t pid;

    // Create the necessary pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process

            // Redirect the input from the previous pipe (if not the first command)
            if (i > 0) {
                if (dup2(pipefd[(i - 1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Redirect the output to the next pipe (if not the last command)
            if (i < num_commands - 1) {
                if (dup2(pipefd[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefd[j]);
            }

            // Execute the command
            execl("/bin/sh", "sh", "-c", commands[i], (char *) NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
    }

    // Close all pipe file descriptors in the parent
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefd[i]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

int main() {
    char *commands[MAX_COMMANDS];
    char command_line[MAX_COMMAND_LENGTH];
    int num_commands = 0;

    // Read commands from stdin
    while (fgets(command_line, MAX_COMMAND_LENGTH, stdin) != NULL && num_commands < MAX_COMMANDS) {
        command_line[strcspn(command_line, "\n")] = '\0';  // Remove newline character
        commands[num_commands] = strdup(command_line);  // Duplicate the command string
        num_commands++;
    }

    // Execute the commands in a pipeline
    execute_commands(num_commands, commands, 0, NULL);

    // Free the allocated memory for the commands
    for (int i = 0; i < num_commands; i++) {
        free(commands[i]);
    }

    return 0;
}
```
# Prova 2023-01-23
#pipe #signals 

## Func
- `mkfifo` -> create pipe with permission (usually 0666)
	- no special bits
	- -rw for 3 fields
- `kill` -> send signal
- `unlink` -> opposite mkfifo
## esercizio es1
```c
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    printf("Usage: %s <fifo_path>\n", argv[0]);
    return 1;
  }

  int fd;

  // FIFO file path
  char * myfifo = argv[1];

  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);

  char arr1[80], arr2[80];
  while (1)
  {
    // Open FIFO for Read only
    fd = open(myfifo, O_RDONLY);

    // Read from FIFO
    read(fd, arr1, sizeof(arr1));

    // Print the read message
    printf("User2: %s\n", arr1);
    close(fd);
    if (strncmp(arr1, "FINE", 4) == 0)
    {
      printf("User1 Exiting...\n");
      break;
    }
  }

  return 0;
}
```
## esercizio es2
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    printf("Usage: %s <fifo_path>\n", argv[0]);
    return 1;
  }

  int fd;

  // FIFO file path
  char * myfifo = argv[1];

  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);

  char arr1[80];
  pid_t proc;
  int signal;
  while (1)
  {
    // Open FIFO for Read only
    fd = open(myfifo, O_RDONLY);

    // Read from FIFO
    read(fd, arr1, sizeof(arr1));

    // Print the read message
    printf("User2: %s\n", arr1);

    close(fd);
    if (strncmp(arr1, "FINE", 4) == 0)
    {
      printf("User1 Exiting...\n");
      break;
    }

    sscanf(arr1, "%d %d", &proc, &signal);

    // send signal to process
    int killr = kill(proc, signal);
    if (killr == 0)
      printf("Signal sent to process %d\n", proc);
    else
      fprintf(stderr,"Error sending signal to process %d\n", proc);
  }

  unlink(myfifo);

  return EXIT_SUCCESS;
}
```
## test 2
```bash
#!/bin/bash

NPP='/tmp/ntmpfifo' # named pipe name

# Define a function to handle the signal
handle_signal() {
    echo "Signal received"
    exit 0
}

# Set up the trap for SIGUSR1
trap 'handle_signal' SIGUSR1

# write my pid and signal to the pipe
echo "123 $$ 10" > $NPP  # SIGUSR1 is signal number 10

# Wait indefinitely for the signal
for i in {1..10}; do
    sleep 1
done

# If the trap catches the signal, it will exit the script
echo "Signal not received"


# #!/bin/bash
#
# NPP='/tmp/ntmpfifo' # named pipe name
#
# # write my pid to the pipe
# echo "$$ SIGUSR1" > $NPP
#
# sleep 1
#
# # check if the signal received is SIGUSR1
# if [ $? -eq 0 ]; then
#     echo "Signal received"
# else
#     echo "Signal not received"
# fi
#
```
## esercizio es3
```python
import os
import sys
import shutil

# Check if the correct number of arguments is provided
if len(sys.argv) != 5:
    print("Usage: python script.py <source_directory> <destination_directory>")
    sys.exit(1)

cfiles = [] # all file paths with a template for the of the first dir
src_dir, dest_dir = sys.argv[1:3]
new_dir1,new_dir2 = sys.argv[3:5]

def main():
  print(new_dir1)
  for root, _, files in os.walk(src_dir):
    for file in files:
      path = os.path.join(root, file).replace(src_dir, '__template__')
      cfiles.append(path)
  
  print('creating new dir')
  os.mkdir(new_dir2)

  print('creating new dir')
  os.mkdir(new_dir1)

  for root, _, files in os.walk(dest_dir):
    for file in files:
      path = os.path.join(root, file)
      generalpath = path.replace(dest_dir, '__template__')
      if generalpath in cfiles: # si puo' ottimizzare con array sortato
        print(f'copia file {file} in {new_dir1} {new_dir2}')
        shutil.copy(generalpath.replace('__template__', src_dir), generalpath.replace('__template__', new_dir1))
        shutil.copy(path, generalpath.replace('__template__', new_dir2))


if __name__ == '__main__':
  main()

```
# Prova 2023-02-16
#files #read #seek #process #fork
## Func
- `ftell` -> tell size in bytes of the position of the fd
- `fseek`
- `sem_init/sem_post/sem_wait/sem_destroy`
- `wait`

>[!danger] Atomicita'
>`pread/pwrite` -> risolvere problema off-set


>[!info] x copia veloce
>utilizzare `open` -> no `fopen` per copie veloci, sono chiamate direttamente all'SO
## esercizio es1
```c
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

sem_t file_sem;

#define MAXBUF 2048
int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s file1 file2\n", argv[0]);
    exit(1);
  }

  FILE *f1 = fopen(argv[1], "r");
  FILE *f2 = fopen(argv[2], "w");

  unsigned numprocs = 2;

  // Initialize the semaphore with a value of 1 (binary semaphore)
  sem_init(&file_sem, 0, 1);

  if (f1 == NULL || f2 == NULL)
  {
    fprintf(stderr, "Error opening files\n");
    exit(1);
  }

  fseek(f1, 0, SEEK_END);
  long fsize = ftell(f1);

  fseek(f1, 0, SEEK_SET);

  char *buffer = malloc(fsize / numprocs);

  int pid = fork();
  if (pid == 0){
    sem_wait(&file_sem);
    fread(buffer, 1, fsize / numprocs, f1);
    fwrite(buffer, 1, fsize / numprocs, f2);
    sem_post(&file_sem);
    exit(0);
  } else if (pid > 0) {
    // parent
    sem_wait(&file_sem);
    fseek(f1, fsize / numprocs, SEEK_SET);
    fread(buffer, 1, fsize / numprocs, f1);
    fwrite(buffer, 1, fsize / numprocs, f2);
    sem_post(&file_sem);
  } else {
    fprintf(stderr, "Error forking\n");
    exit(1);
  }

  int status;
  pid_t child_pid = wait(&status);  // Wait for the child process to finish

  if (child_pid == -1) {
    perror("wait");
    return 1;
  }

  fclose(f1);
  fclose(f2);
  free(buffer);
  sem_destroy(&file_sem);
  
  return EXIT_SUCCESS;
}
```
## esercizio es2
```c
#include <dirent.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

sem_t file_sem;

#define MAXBUF 2048
int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <n copy proc> file1 file2\n", argv[0]);
    exit(1);
  }

  unsigned numprocs = atoi(argv[1]);
  FILE *f1 = fopen(argv[2], "r");
  FILE *f2 = fopen(argv[3], "w");

  // Initialize the semaphore with a value of 1 (binary semaphore)
  sem_init(&file_sem, 0, 1);

  if (f1 == NULL || f2 == NULL) {
    fprintf(stderr, "Error opening files\n");
    exit(1);
  }

  fseek(f1, 0, SEEK_END);
  long fsize = ftell(f1);

  fseek(f1, 0, SEEK_SET);

  int offset = fsize / numprocs;
  int last_offset = fsize % numprocs;
  char *buffer = malloc(offset + 1);

  for (int i = 0; i < numprocs; i++) {
    int pid = fork();
    if (pid == 0) {
      sem_wait(&file_sem);
      fseek(f1, offset * i, SEEK_SET);
      int readc = (i == numprocs - 1) ? (offset + last_offset) : offset;
      buffer = malloc(readc + 1);
      readc = fread(buffer, 1, readc, f1);
      fseek(f2, offset * i, SEEK_SET);
      fwrite(buffer, 1, readc, f2);
      free(buffer);
      sem_post(&file_sem);
      exit(EXIT_SUCCESS);
    } else if (pid < 0) {
      fprintf(stderr, "Error forking\n");
      exit(EXIT_FAILURE);
    }
  }

  pid_t child_pid;
  int status;
  int children_left = numprocs; // Track number of child processes

  // Wait for all child processes to finish
  while ((child_pid = wait(&status)) > 0) {
    children_left--;
    printf("Child process %d finished\n", child_pid);
    if (children_left == 0) {
      break; // All child processes have finished
    }
  }

  if (child_pid == -1) {
    perror("wait");
    return 1;
  }

  fclose(f1);
  fclose(f2);
  free(buffer);
  sem_destroy(&file_sem);

  return EXIT_SUCCESS;
}
```
## esercizio es3
```python
import os
import sys


if len(sys.argv) != 2:
  print(f'Usage: {sys.argv[0]} <dirpath>')
  sys.exit(1)

dirpath = sys.argv[1]
line_char_cache = {}

def findcharnum():
  for root, _, files in os.walk(dirpath):
    for file in files:
      filepath = os.path.join(root, file)
      with open(filepath) as f:
        for linenum, line in enumerate(f, 1):
          line_char_cache[linenum] = line_char_cache.get(linenum, 0) + len(line)
          
def main():
  findcharnum()
  for linenum, charnum in line_char_cache.items():
    print(f'{linenum}: {charnum}')

if __name__ == '__main__':
    main()
```
# Prova 2023-05-24
#process #pid #active #dirs #clone 

## Func
- `opendir, readdir, closedir`
- `sprinf`
- `fread`
- `execve` -> lanciare un processo con shell
- `waitpid`

- `os.path.islink`
- `os.path.realpath`
## esercizio es1
```c
/*
 * Scriveere un programma pidcmd che stampi i pid dei processi attivi lanciati
con una specifica riga di comando. (Devono coincidere tutti gli argomenti) es:
ll comando "pidcmd less /etc/hostname" deve stampare il numero di processo dei
processi attivi che sono stati lanciati con "less /etc/hostname" (hint: cercare
nelle directory dei processi in /proc i "file" chiamati cmdline)
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUF 2048

unsigned findactiveproc(char *cmd) {
  unsigned num = 0;

  char *procdir = "/proc";
  DIR *FD = opendir(procdir);
  struct dirent *de;
  if (FD == NULL) {
    fprintf(stderr, "ERROR finding proc");
    exit(1);
  }

  printf("open /proc\n");

  while (NULL != (de = readdir(FD))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;

    if (de->d_type == DT_DIR) { // find a process
      char *filename =
          malloc(sizeof(char) * (strlen(procdir) + strlen(de->d_name) + 8));
      sprintf(filename, "%s/%s/cmdline", procdir, de->d_name);
      // opening the file info
      FILE *ptr;
      char buf[MAXBUF];

      if (NULL == (ptr = fopen(filename, "r"))) {
        // fprintf(stderr,"ERROR opening file %s\n", filename);
        free(filename);
        continue;
      }

      size_t bytesRead = fread(&buf, 1, MAXBUF - 1, ptr);

      buf[bytesRead] = '\0'; // Ensure null termination

      // Replace null characters with spaces for easier searching
      for (size_t i = 0; i < bytesRead; i++) {
        if (buf[i] == '\0') {
          buf[i] = ' ';
        }
      }
      // Ensure to trim any trailing spaces from the buffer
      char *end = buf + strlen(buf) - 1;
      while (end > buf && *end == ' ') {
        *end = '\0';
        end--;
      }

      if (!strcmp(cmd, buf)) {
        printf("trovato! pid %s", de->d_name);
        num++;
      }

      fclose(ptr);
      free(filename);
    }
  }

  closedir(FD);

  return num;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <cmd> [args...]\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Calculate the total length for the command string
  size_t len = 0;
  for (int i = 1; i < argc; i++) {
    len += strlen(argv[i]) + 1; // +1 for spaces or null terminator
  }

  char *cmd = malloc(len);
  if (cmd == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return EXIT_FAILURE;
  }

  strcpy(cmd, argv[1]);
  for (int i = 2; i < argc; i++) {
    strcat(cmd, " ");
    strcat(cmd, argv[i]);
  }

  unsigned nproc = findactiveproc(cmd);
  printf("Active processes: %u\n", nproc);

  free(cmd);
  return EXIT_SUCCESS;
}
```
## esercizio es2
```c
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXBUF 2048
#define SHELL "/bin/sh"

void parsestr(char *str, int len, char old, char new) {
  for (int i = 0; i < len; i++) {
    if (str[i] == old) {
      str[i] = new;
    }
  }
}

void cloneproc(char *nproc) {
  char *procdir = "/proc";
  DIR *FD = opendir(procdir);
  struct dirent *de;
  if (FD == NULL) {
    fprintf(stderr, "ERROR finding proc");
    exit(EXIT_FAILURE);
  }

  printf("open /proc\n");

  while (NULL != (de = readdir(FD))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;

    if (de->d_type == DT_DIR && !strcmp(de->d_name, nproc)) { // find a process
      char cfile[MAXBUF]; // cmdline file 
      snprintf(cfile, MAXBUF, "%s/%s/cmdline", procdir, de->d_name);
      char efile[MAXBUF]; // environ file
      snprintf(efile, MAXBUF, "%s/%s/environ", procdir, de->d_name);
      
      FILE *cmdline = fopen(cfile, "r");
      FILE *environ = fopen(efile, "r");

      if (cmdline == NULL || environ == NULL) {
        fprintf(stderr, "ERROR opening cmdline or environ file\n");
        exit(EXIT_FAILURE);
      }

      char cmd[MAXBUF];
      char env[MAXBUF];
      unsigned long bytes_read = 0;
      
      // read cmdline and environ files
      if ((bytes_read = fread(cmd, 1, MAXBUF, cmdline)) == -1) {
        fprintf(stderr, "ERROR reading cmdline file\n");
        exit(EXIT_FAILURE);
      }

      parsestr(cmd, bytes_read, '\0', ' ');

      cmd[bytes_read] = '\0';

      printf("cmd: %s\n", cmd);

      if ((bytes_read = fread(env, 1, MAXBUF, environ)) == -1) {
        fprintf(stderr, "ERROR reading environ file\n");
        exit(EXIT_FAILURE);
      }

      parsestr(env, bytes_read, '\0', ' ');
      printf("env: %s\n", env);

      // close files
      fclose(cmdline);
      fclose(environ);

      // set the right cwd
      char cwd[MAXBUF];
      snprintf(cwd, MAXBUF, "/proc/%s/cwd", de->d_name);
      if (chdir(cwd) == -1) {
        fprintf(stderr, "ERROR changing directory\n");
        exit(EXIT_FAILURE);
      }

      pid_t pid = fork();
      if (pid == -1) {
        fprintf(stderr, "ERROR forking\n");
        exit(EXIT_FAILURE);
      }

      if (pid != 0) { // parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
          printf("Child exited with status %d\n", WEXITSTATUS(status));
        }
        
		closedir(FD);
        return;
      }

      // executing the new command
      char *args[] = {SHELL, "-c", cmd, NULL};
      char *envs[] = {env, NULL};
      if (execve(SHELL, args, envs) == -1) {
        fprintf(stderr, "ERROR executing command\n");
        exit(EXIT_FAILURE);
      }

      exit(EXIT_SUCCESS); // technically unreachable
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <pid proc>\n", argv[0]);
    return EXIT_FAILURE;
  }

  cloneproc(argv[1]);

  return EXIT_SUCCESS;
}
```
## esercizio es3
```python
import os
import sys

cache = [] # file linked

def arecommonsymlink(dest):
  for root, dirs, files in os.walk(dest):
    for file in files:
      filename = os.path.join(root,file)
      if os.path.islink(filename):
        res_ln = os.path.realpath(filename)
        if res_ln in cache:
          return True
        cache.append(res_ln)
  return False

if(len(sys.argv) != 2 or not os.path.isdir(sys.argv[1])):
  print("ERROR: usage $ python3 script.py path/to/dir")
  exit(1)

if arecommonsymlink(sys.argv[1]):
  print("ci sono")
else:
  print("no")
```
# Prova 2023-06-14
#intervals #regular #timer 

## Func
- `timerfd_create`
- `timerfd_settime`
	- `new_value.it_value.* && new_value.it_interval.*` -> set dei valori
- `fds[0].fd = timer_fd; fds[0].events = POLLIN;` -> polling event
## esercizio es1
```c
/*Facendo uso dei timerfd (vedi timerfd_create) scrivere un programma che stampi
una stringa a intervalli regolari. (il parametro ha tre campi separati da
virgola: il numero di iterazioni, l'intervallo fra iterazione e la stringa da
salvare:
 */
#include <bits/time.h>
#include <dirent.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define MAXBUF 2048

int intervalprint(int ntimes, float interval, char *str){
  int timer_fd;
  struct itimerspec new_value;
  struct pollfd fds[1];
  uint64_t exp;
  struct timespec start_time, current_time;

  // Create the timer file descriptor
  timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (timer_fd == -1) {
    perror("timerfd_create");
    return EXIT_FAILURE;
  }

  // Configure the timer
  new_value.it_value.tv_sec = (int)interval;
  new_value.it_value.tv_nsec = (interval - (int)interval) * 1000000000;
  new_value.it_interval.tv_sec = (int)interval;
  new_value.it_interval.tv_nsec = (interval - (int)interval) * 1000000000;

  if (timerfd_settime(timer_fd, 0, &new_value, NULL) == -1) {
    perror("timerfd_settime");
    close(timer_fd);
    return EXIT_FAILURE;
  }

  fds[0].fd = timer_fd;
  fds[0].events = POLLIN;

  printf("Timer started. Printing string \"%s\" every %.2f seconds for %d "
         "times.\n",
         str, interval, ntimes);
  
  // Get the start time
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  // Poll for timer events and print the string
  for (int i = 0; i < ntimes; i++) {
    int ret = poll(fds, 1, -1);
    if (ret == -1) {
      perror("poll");
      close(timer_fd);
      return EXIT_FAILURE;
    }

    if (fds[0].revents & POLLIN) {
      ssize_t s = read(timer_fd, &exp, sizeof(uint64_t));
      if (s != sizeof(uint64_t)) {
        perror("read");
        close(timer_fd);
        return EXIT_FAILURE;
      }
      // Get the current time
      clock_gettime(CLOCK_MONOTONIC, &current_time);

      // Calculate the elapsed time
      double elapsed_time =
          (current_time.tv_sec - start_time.tv_sec) +
          (current_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

      // Print the elapsed time and the string
      printf("Elapsed time: %.2f seconds - %s\n", elapsed_time, str);
    }
  }

  close(timer_fd);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  // $ tfdtest 4,1.1,ciao
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <num_iterations>,<interval>,<string>\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  int ntimes;
  float interval;
  char str[256];

  // Parse the input arguments
  if (sscanf(argv[1], "%d,%f,%255[^\n]", &ntimes, &interval, str) != 3) {
    fprintf(stderr, "Invalid input format. Expected "
                    "<num_iterations>,<interval>,<string>\n");
    return EXIT_FAILURE;
  }

  return intervalprint(ntimes, interval, str);
}
```
## esercizio es2
```c
/*Facendo uso dei timerfd (vedi timerfd_create) scrivere un programma che stampi
una stringa a intervalli regolari. (il parametro ha tre campi separati da
virgola: il numero di iterazioni, l'intervallo fra iterazione e la stringa da
salvare:
Estendere l'esercizio 1 in modo che gestisca molteplici timer:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <poll.h>
#include <stdint.h> // for uint64_t
#include <time.h>   // for clock_gettime
#include <sys/wait.h>

#define MAXBUF 2048

int intervalprint(int ntimes, float interval, char *str) {
  int timer_fd;
  struct itimerspec new_value;
  struct pollfd fds[1];
  uint64_t exp;
  struct timespec start_time, current_time;

  // Create the timer file descriptor
  timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (timer_fd == -1) {
    perror("timerfd_create");
    exit(EXIT_FAILURE);
  }

  // Configure the timer
  new_value.it_value.tv_sec = (int)interval;
  new_value.it_value.tv_nsec = (interval - (int)interval) * 1000000000;
  new_value.it_interval.tv_sec = (int)interval;
  new_value.it_interval.tv_nsec = (interval - (int)interval) * 1000000000;

  if (timerfd_settime(timer_fd, 0, &new_value, NULL) == -1) {
    perror("timerfd_settime");
    close(timer_fd);
    exit(EXIT_FAILURE);
  }

  fds[0].fd = timer_fd;
  fds[0].events = POLLIN;

  printf("Timer started. Printing string \"%s\" every %.2f seconds for %d "
         "times.\n",
         str, interval, ntimes);

  // Get the start time
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  // Poll for timer events and print the string
  for (int i = 0; i < ntimes; i++) {
    int ret = poll(fds, 1, -1);
    if (ret == -1) {
      perror("poll");
      close(timer_fd);
      exit(EXIT_FAILURE);
    }
q
    if (fds[0].revents & POLLIN) {
      ssize_t s = read(timer_fd, &exp, sizeof(uint64_t));
      if (s != sizeof(uint64_t)) {
        perror("read");
        close(timer_fd);
        exit(EXIT_FAILURE);
      }
      // Get the current time
      clock_gettime(CLOCK_MONOTONIC, &current_time);

      // Calculate the elapsed time
      double elapsed_time =
          (current_time.tv_sec - start_time.tv_sec) +
          (current_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

      // Print the elapsed time and the string
      printf("Elapsed time: %.2f seconds - %s\n", elapsed_time, str);
    }
  }

  close(timer_fd);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  // $ tfdtest 4,1.1,ciao
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <num_iterations>,<interval>,<string> ... \n",
            argv[0]);
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    int pid = fork();
    if (pid == 0) {

      int ntimes;
      float interval;
      char str[256];

      // Parse the input arguments
      if (sscanf(argv[i], "%d,%f,%255[^\n]", &ntimes, &interval, str) != 3) {
        fprintf(stderr, "Invalid input format. Expected "
                        "<num_iterations>,<interval>,<string>\n");
        return EXIT_FAILURE;
      }

      intervalprint(ntimes, interval, str);
    } else if (pid > 0) {
      continue;
    } else if (pid < 0) {
      perror("fork");
      return EXIT_FAILURE;
    }
  }

  // Wait for all child processes to complete
  while (wait(NULL) > 0);

  return EXIT_SUCCESS;
}
```
# Prova 2023-07-20
#clene #dirs #link

## Func
- `link` 
- `sprinf`
- `realpath` -> obtain realpath of a file
- `stat` -> obtain file statistics

- `all` -> check if all element in a list are True
- `os.path.isdir`
## esercizio es1
```c
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXBUF 4096

int clonedir(char *src, char *dst) {
  DIR *FROM;
  if (NULL == (FROM = opendir(src))) {
    fprintf(stderr, "ERR from dir\n");
  }

  // controllo esistenza directory dei link
  struct stat st = {0};

  if (stat(dst, &st) == -1) {
      mkdir(dst, 0700);
  }

  struct dirent *in_filef;
  while (NULL != (in_filef = readdir(FROM))) {
    if(!strcmp(in_filef->d_name, ".") || !strcmp(in_filef->d_name, "..")){
      continue;
    }

    if (in_filef->d_type == DT_DIR) {
      printf("recursive clone %s\n", in_filef->d_name);
      char *newsrc = malloc(sizeof(src) + sizeof(in_filef->d_name) + 4);
      char *newdst = malloc(sizeof(dst) + sizeof(in_filef->d_name) + 4);
      
      sprintf(newsrc, "%s/%s", src, in_filef->d_name);
      sprintf(newdst, "%s/%s", dst, in_filef->d_name);

      clonedir(newsrc, newdst);

      free(newdst);
      free(newsrc);
    }
    char *original_file = malloc(sizeof(src) + sizeof(in_filef->d_name) + 4);
    char *new_link= malloc(sizeof(src) + sizeof(in_filef->d_name) + 4);

    sprintf(original_file, "%s/%s", src, in_filef->d_name);
    sprintf(new_link, "%s/%s", dst, in_filef->d_name);

    if(link(original_file, new_link) == -1){
      fprintf(stderr, "ERROR: linking %s\n", original_file);
    } else {
      printf("linked %s\n", original_file);
    }

    free(original_file);
    free(new_link);
  }

  return 0;
}

int main(int argc, char **argv) {
  char *src = argv[1];
  char *dst = argv[2];

  clonedir(realpath(src, NULL), realpath(dst, NULL));

  return 0;
}
```
## esercizio es2
```c
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAXBUF 4096

int clonedir(char *src, char *dst, long time) {
  DIR *FROM;
  if (NULL == (FROM = opendir(src))) {
    fprintf(stderr, "ERR from dir\n");
  }

  // controllo esistenza directory dei link
  struct stat st = {0};

  if (stat(dst, &st) == -1) {
    mkdir(dst, 0700);
  }

  struct dirent *in_filef;
  while (NULL != (in_filef = readdir(FROM))) {
    if (!strcmp(in_filef->d_name, ".") || !strcmp(in_filef->d_name, "..")) {
      continue;
    }

    if (in_filef->d_type == DT_DIR) {
      printf("recursive clone %s\n", in_filef->d_name);
      char *newsrc = malloc(sizeof(src) + sizeof(in_filef->d_name) + 4);
      char *newdst = malloc(sizeof(dst) + sizeof(in_filef->d_name) + 4);

      sprintf(newsrc, "%s/%s", src, in_filef->d_name);
      sprintf(newdst, "%s/%s", dst, in_filef->d_name);

      clonedir(newsrc, newdst, time);

      free(newdst);
      free(newsrc);
      continue;
    }
    char *original_file = malloc(sizeof(src) + sizeof(in_filef->d_name) + 4);
    char *new_f = malloc(sizeof(src) + sizeof(in_filef->d_name) + 4);

    sprintf(original_file, "%s/%s", src, in_filef->d_name);
    sprintf(new_f, "%s/%s", dst, in_filef->d_name);

    // check last modification time
    struct stat st_file;
    if (stat(original_file, &st_file) != 0) {
      fprintf(stderr, "stat file %s", original_file);
      return 1;
    }

    if (st.st_mtim.tv_sec < time) { // then link
      if (link(original_file, new_f) == -1) {
        fprintf(stderr, "ERROR: linking %s\n", original_file);
      } else {
        printf("linked %s\n", original_file);
      }
    } else { // copia
      int fd_from = open(original_file, O_RDONLY);
      if (fd_from < 0) {
        fprintf(stderr, "ERROR opening %s\n", original_file);
        exit(1);
      }
      int fd_to = open(new_f, O_WRONLY | O_CREAT | O_EXCL, 0666);
      if (fd_to < 0) {
        fprintf(stderr, "ERROR: creating new file %s\n", new_f);
        goto close;
      }

      ssize_t nread;
      char buf[4096];
      while (nread = read(fd_from, buf, sizeof(buf))) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
          nwritten = write(fd_to, out_ptr, nread);

          if (nwritten >= 0) {
            nread -= nwritten;
            out_ptr += nwritten;
          }
        } while (nread > 0);
      }

      printf("copia effettuata con successo %s -> %s\n", original_file, new_f);
      close:
      close(fd_from);
      close(fd_to);
    }

    free(original_file);
    free(new_f);
  }

  closedir(FROM);

  return 0;
}

int main(int argc, char **argv) {
  char *src = argv[2];
  char *dst = argv[3];
  char *endptr;
  long time = strtol(argv[1], &endptr, 10);


  clonedir(realpath(src, NULL), realpath(dst, NULL), time);

  return 0;
}
```
## esercizio es3
```python
import os
import sys

def is_ascii(s):
    return all(ord(c) < 128 for c in s)


def findnotascii(path):
  res = []
  for root, dirs, files in os.walk(path):
    for file in files + dirs:
      if not is_ascii(file):
        res.append(file) # maybe os.path.join(root, file)
  return res

def main():
  if len(sys.argv) != 2:
    print("Usage: python script.py <directory_path>")
    sys.exit(1)

  fd = sys.argv[1]

  if not os.path.isdir(fd):
    print(f"Error: {fd} is not a valid directory")
    sys.exit(1)

  notascii = findnotascii(os.path.abspath(fd))
  print(notascii)

if __name__ == "__main__":
    main()
```
# Prova 2023-09-14
#list #files #dirs #copy
## esercizio es1
```c
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

int countfile(DIR *FD){
  struct dirent* entry;
  int file_count = 0;
  while ((entry = readdir(FD)) != NULL) {
        // Skip the "." and ".." entries
        if (entry->d_name[0] != '.' && entry->d_type != DT_DIR) {
            file_count++;
        }
    }
  return file_count;
}

char **vreaddir(const char *path){
  DIR* FD;
  struct dirent* in_file;

  if(NULL == (FD = opendir(path))){
    fprintf(stderr, "ERROR: Failed to opend directory(%s)!", path);
    exit(1);
  }

  int fc = countfile(FD);
  printf("fc:%d\n\n", fc);
  closedir(FD); FD = opendir(path);

  int i = 0;
  char **files = malloc(sizeof(char*) * (fc + 1));

  while((in_file = readdir(FD))){
    if(!strcmp(in_file->d_name, ".."))
      continue;
    if(!strcmp(in_file->d_name, "."))
      continue;
    
    if(in_file->d_type != DT_DIR){
      files[i] = malloc(sizeof(char) * strlen(in_file->d_name));
      strcpy(files[i++], in_file->d_name);
    }
  }

  closedir(FD);
  files[i] = NULL;
  return files;
}

int main(int argc, char** argv){
  char** files_from_dir = vreaddir(argv[1]);

  char* curr = NULL; int i = 0;
  while(NULL != (curr = files_from_dir[i++])){
    printf("file %d: %s\n", i, curr);
  }

  return 0;
}
```
## esercizio es3

```python
import os
import sys

def main():
    # get the path
    subtree = sys.argv[1]
    path = os.path.abspath(subtree)
    cache = {}

    for root, dirs, files in os.walk(subtree):
        for name in files:
            subpath = os.path.join(root,name)
            print(subpath)
            if os.path.islink(subpath):
                resolved_link = os.path.realpath(subpath)
                if resolved_link not in cache:
                    cache[resolved_link] = []
                cache[resolved_link].append(os.path.abspath(subpath))

    print(cache)


if __name__ == "__main__":
    main()
```
# Prova 2024-01-19
#format #string #pipe 
## Func
- `execvp`
- `memcpy`
- `write`
## esercizio es1a
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *unformatstr(char *str, int len) {
  for (int i = 0; i < len; i++) {
    if (str[i] == '\0')
      str[i] = ' ';
  }
  return str;
}
int main(int argc, char *argv[]) {
  char buffer[1024];
  ssize_t bytes_read;
  printf("miao");

  // Read from standard input until EOF
  bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

  if (bytes_read < 0) {
    perror("read");
    return EXIT_FAILURE;
  }
  printf("s-%s", buffer);

  char *fstr = unformatstr(buffer, bytes_read);
  printf("%s", fstr);

  // Prepare arguments for execlp
  char *args[] = {"/bin/sh", "-c", fstr, NULL};
  int res = execvp(args[0], args);

  // If execvp returns, there was an error
  perror("execvp");
  return EXIT_FAILURE;
}
```
## esercizio es1b
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SPACESEPARATOR 2

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <args>\n", argv[0]);
    return EXIT_FAILURE;
  }
 
  char *fstr;
  int maxsize = 0;
  for (int i = 1; i < argc; i++) {
    maxsize += strlen(argv[i]);
  }

  maxsize += (argc - 1); // num of \0

  fstr = malloc(maxsize * sizeof(char));
  if (fstr == NULL) {
    fprintf(stderr, "Error allocating string");
  }

  int pos = 0;
  for (int i = 1; i < argc; i++) {
    int len = strlen(argv[i]);
    memcpy(fstr + pos, argv[i], len);
    pos += len;
    fstr[pos++] = '\0';
  }

  // Write the result to standard output
  if (write(STDOUT_FILENO, fstr, maxsize) != maxsize) {
    perror("write");
    free(fstr);
    return EXIT_FAILURE;
  }

  free(fstr);

  return EXIT_SUCCESS;
}
```
## esercizio es2
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

char *unformatstr(char *str, int len) {
    for (int i = 0; i < len; i++) {
        if (str[i] == '\0')
            str[i] = ' ';
    }
    return str;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fifo_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd;
    char *fifo_path = argv[1];

    // Create the named pipe
    if (mkfifo(fifo_path, 0666) == -1) {
        perror("mkfifo");
        return EXIT_FAILURE;
    }

    char buffer[1024];
    ssize_t bytes_read;

    // Open the named pipe for reading
    fd = open(fifo_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        unlink(fifo_path);
        return EXIT_FAILURE;
    }

    // Read from the named pipe
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("read");
        close(fd);
        unlink(fifo_path);
        return EXIT_FAILURE;
    }
    buffer[bytes_read] = '\0'; // Null-terminate the buffer

    close(fd);
    unlink(fifo_path);

    // Convert null terminators to spaces
    char *fstr = unformatstr(buffer, bytes_read);

    // Debugging: Print the received command
    printf("Executing command: %s\n", fstr);

    // Prepare arguments for execlp
    char *args[] = {"/bin/sh", "-c", fstr, NULL};
    int res = execvp(args[0], args);

    // If execvp returns, there was an error
    perror("execvp");
    return EXIT_FAILURE;
}
```
