#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

pid_t monitor_pid = -1;
int monitor_running = 0;

typedef enum
  {
    START_MONITOR,
    LIST_HUNTS,
    LIST_TREASURES,
    VIEW_TREASURE,
    STOP_MONITOR,
    EXIT,
    CALCULATE_SCORE,
    UNKNOWN
  }Command;

Command parse(char *comanda)
{
  if((strcmp(comanda, "start_monitor")) == 0)
    return START_MONITOR;
  if((strcmp(comanda, "list_hunts")) == 0)
    return LIST_HUNTS;
  if((strcmp(comanda, "list_treasures")) == 0)
    return LIST_TREASURES;
  if((strcmp(comanda, "view_treasure")) == 0)
    return VIEW_TREASURE;
  if((strcmp(comanda, "stop_monitor")) == 0)
    return STOP_MONITOR;
  if((strcmp(comanda, "exit")) == 0)
    return EXIT;
  if((strcmp(comanda, "calculate_score")) == 0)
    return CALCULATE_SCORE;
  return UNKNOWN;
}

void send_signal(pid_t pid, Command cmd)
{
  if(pid <= 0)
    {
      printf("Monitorul probabil nu e pornit\n");
      return;
    }
  
  switch(cmd)
    {
    case LIST_HUNTS:
      {
	if(kill(pid, SIGUSR1) == -1)
	    perror("Eroare la trimiterea SIGUSR1 (list_hunts)\n");
	else
	    printf("S-a trimis semnalul SIGUSR1 (list_hunts)\n)");
	break;
      }

    case LIST_TREASURES:
      {
	if(kill(pid, SIGUSR2) == -1)
	  perror("Eroare la trimiterea SIGUSR2 (list_treasures)\n");
	else
	  printf("S-a trimis semnalul SIGUSR2 (list_treasures)\n");
	break;
      }

    case VIEW_TREASURE:
      {
	if(kill(pid, SIGINT) == -1)
	  perror("Eroare la trimiterea semnalului SIGINT (view_treasure)\n");
	else
	  printf("S-a trimis semnalul SIGINT (view_treasure)\n");
	break;
      }

    case STOP_MONITOR:
      {
	if(kill(pid, SIGTERM) == -1)
	  perror("Eroare la trimiterea semnalului SIGTERM (stop_monitor)\n");
	else
	  printf("S-a trimis semnalul SIGTERM (stop_monitor)\n");
	break;
      }

    default:
      {
	printf("Comanda necunoscuta\n");
	break;
      }
    }
}

void calculate_scores() {
  DIR *dir = opendir(".");   //directorul actual
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        struct stat st;
        if (lstat(entry->d_name, &st) == -1)
            continue;

        if (S_ISLNK(st.st_mode))
            continue;

        //doar directoare
        if (!S_ISDIR(st.st_mode))
            continue;

        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(fd[0]); close(fd[1]);
            continue;
        }

        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);

            execlp("./score_calculator", "score_calculator", entry->d_name, NULL);
            perror("execlp");
            exit(1);
        } else {
            close(fd[1]);
            printf("Scores for hunt: %s\n", entry->d_name);

            char buffer[256];
            ssize_t bytes;
            while ((bytes = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes] = '\0';
                printf("%s", buffer);
            }
            close(fd[0]);
            waitpid(pid, NULL, 0);
            printf("\n");
        }
    }
    closedir(dir);
}

void start_monitor()
{
  monitor_running = 1;
  if(monitor_pid != -1)
    {
      printf("Monitorul ruleaza deja\n");
      return;
    }

  pid_t pid = fork();

  if(pid < 0)
    {
      printf("Eroare la fork\n");
    }
  else if(pid == 0)
    {
      execl("./monitor", "monitor", NULL);
      perror("eroare la execl");
      exit(-1);
    }
  else
    {
      monitor_pid = pid;
      printf("Monitorul s-a pornit cu pid-ul: %d\n", monitor_pid);
    }
}

void handle_sigchld(int sig) {
    int status;
    pid_t pid = waitpid(monitor_pid, &status, WNOHANG);
    if (pid == monitor_pid) {
        printf("[treasure_hub] Monitorul s-a inchis complet (SIGCHLD)\n");
        monitor_pid = -1; 
    }
}

void setup_handlers() {
    struct sigaction sa;

    // Handler pentru SIGCHLD
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);
}

int main()
{
  char buffer[256];

  while(1)
  {
      printf(">> ");
      fgets(buffer, sizeof(buffer), stdin);
      buffer[strcspn(buffer, "\n")] = '\0';
     
      Command cmd = parse(buffer);
      switch(cmd)
	{
	case START_MONITOR:
	  {
	    start_monitor();
	    break;
	  }

	case LIST_HUNTS:
	  {
	    if(monitor_running)
	      send_signal(monitor_pid, LIST_HUNTS);
	    else
	      printf("EROARE: Monitorul nu e pornit si nu mai primeste comenzi\n");
	    break;
	  }

	case LIST_TREASURES:
	  {
	    if(monitor_running)
	      send_signal(monitor_pid, LIST_TREASURES);
	    else
	       printf("EROARE: Monitorul nu e pornit si nu mai primeste comenzi\n");
	    break;
	  }

	case VIEW_TREASURE:
	  {
	    if(monitor_running)
	      send_signal(monitor_pid, VIEW_TREASURE);
	    else
	      printf("EROARE: Monitorul nu e pornit si nu mai primeste comenzi\n");
	  }

	case STOP_MONITOR:
	  {
	    monitor_running = 0;
	    send_signal(monitor_pid, STOP_MONITOR);
	    break;
	  }

	case EXIT:
	    {
	      send_signal(monitor_pid, STOP_MONITOR);
	      exit(0);
	      break;
	    }

	case CALCULATE_SCORE:
	  {
	    calculate_scores();
	    break;
	  }

	case UNKNOWN:
	default:
	  {
	    printf("\nComanda necunoscuta\n");
	    break;
	  }	  
	}
      }
  return 0;
}
