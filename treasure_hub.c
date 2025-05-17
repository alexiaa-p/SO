/*#include <stdio.h>
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

void calculate_score(const char *hunt_dir) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execl("./score_calculator", "score_calculator", hunt_dir, (char *)NULL);
        perror("execl");
        exit(1);
    } else {
        close(pipefd[1]);

        char buffer[256];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);
        }

        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
    }
}
*/


/*void start_monitor()
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
    }*/

/*void start_monitor()
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        printf("Eroare la fork\n");
        return;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execl("./monitor", "monitor", NULL);
        perror("eroare la execl");
        exit(-1);
    } else {
        close(pipefd[1]);
        monitor_pid = pid;
        monitor_running = 1;
        printf("Monitorul s-a pornit cu pid-ul: %d\n", monitor_pid);

        FILE *monitor_output = fdopen(pipefd[0], "r");
        if (!monitor_output) {
            perror("fdopen");
            return;
        }

        char line[256];
        while (fgets(line, sizeof(line), monitor_output)) {
            printf("[Monitor] %s", line);
        }

        fclose(monitor_output);
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
	    char hunt_id[64];
	    printf("Hunt ID: ");
	    fgets(hunt_id, sizeof(hunt_id), stdin);
	    hunt_id[strcspn(hunt_id, "\n")] = '\0';

	    calculate_score(hunt_id);
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
  }*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/select.h>

pid_t monitor_pid = -1;
int monitor_running = 0;
int monitor_pipe_fd = -1;
int monitor_pipe_write_fd = -1;

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
} Command;

Command parse(char *comanda)
{
    if (strcmp(comanda, "start_monitor") == 0)
        return START_MONITOR;
    if (strcmp(comanda, "list_hunts") == 0)
        return LIST_HUNTS;
    if (strcmp(comanda, "list_treasures") == 0)
        return LIST_TREASURES;
    if (strcmp(comanda, "view_treasure") == 0)
        return VIEW_TREASURE;
    if (strcmp(comanda, "stop_monitor") == 0)
        return STOP_MONITOR;
    if (strcmp(comanda, "exit") == 0)
        return EXIT;
    if (strcmp(comanda, "calculate_score") == 0)
        return CALCULATE_SCORE;
    return UNKNOWN;
}

void send_signal(pid_t pid, Command cmd)
{
    if (pid <= 0)
    {
        printf("Monitorul probabil nu e pornit\n");
        return;
    }

    switch (cmd)
    {
    case LIST_HUNTS:
        if (kill(pid, SIGUSR1) == -1)
            perror("Eroare la trimiterea SIGUSR1 (list_hunts)\n");
        else
            printf("S-a trimis semnalul SIGUSR1 (list_hunts)\n");
        break;

    case LIST_TREASURES:
        if (kill(pid, SIGUSR2) == -1)
            perror("Eroare la trimiterea SIGUSR2 (list_treasures)\n");
        else
            printf("S-a trimis semnalul SIGUSR2 (list_treasures)\n");
        break;

    case VIEW_TREASURE:
        if (kill(pid, SIGINT) == -1)
            perror("Eroare la trimiterea semnalului SIGINT (view_treasure)\n");
        else
            printf("S-a trimis semnalul SIGINT (view_treasure)\n");
        break;

    case STOP_MONITOR:
        if (kill(pid, SIGTERM) == -1)
            perror("Eroare la trimiterea semnalului SIGTERM (stop_monitor)\n");
        else
            printf("S-a trimis semnalul SIGTERM (stop_monitor)\n");
        break;

    default:
        printf("Comanda necunoscuta\n");
        break;
    }
}

void calculate_score(const char *hunt_dir)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execl("./score_calculator", "score_calculator", hunt_dir, (char *)NULL);
        perror("execl");
        exit(1);
    }
    else
    {
        close(pipefd[1]);

        char buffer[256];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[n] = '\0';
            printf("%s", buffer);
        }

        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
    }
}

void start_monitor()
{
    if (monitor_running)
    {
        printf("Monitorul ruleaza deja\n");
        return;
    }

    int pipefd_out[2]; // monitor -> treasure_hub (stdout monitor -> read treasure_hub)
    int pipefd_in[2];  // treasure_hub -> monitor (stdin monitor -> read monitor)

    if (pipe(pipefd_out) == -1 || pipe(pipefd_in) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        // copil - monitor

        // inchidem capetele nefolosite
        close(pipefd_out[0]);  // inchidem capatul de citire monitor->hub
        close(pipefd_in[1]);   // inchidem capatul de scriere hub->monitor

        // redirectionam stdin si stdout
        dup2(pipefd_in[0], STDIN_FILENO);
        close(pipefd_in[0]);

        dup2(pipefd_out[1], STDOUT_FILENO);
        close(pipefd_out[1]);

        execl("./monitor", "monitor", NULL);
        perror("execl monitor");
        exit(1);
    }
    else
    {
        // parinte - treasure_hub

        // inchidem capetele nefolosite
        close(pipefd_out[1]);  // inchidem capatul de scriere monitor->hub
        close(pipefd_in[0]);   // inchidem capatul de citire hub->monitor

        monitor_pid = pid;
        monitor_running = 1;
        monitor_pipe_fd = pipefd_out[0];       // citire din monitor
        monitor_pipe_write_fd = pipefd_in[1];  // scriere catre monitor

        printf("Monitorul s-a pornit cu pid-ul: %d\n", monitor_pid);
    }
}

void send_command_with_args(const char *args, Command cmd)
{
    if (monitor_pipe_write_fd == -1)
    {
        printf("Monitorul nu este pornit\n");
        return;
    }

    // Scriem parametrii in pipe catre monitor
    write(monitor_pipe_write_fd, args, strlen(args));
    write(monitor_pipe_write_fd, "\n", 1); // terminam cu newline

    // Apoi trimitem semnalul ca sa-l trezim sa proceseze comanda
    send_signal(monitor_pid, cmd);
}

void check_monitor_output()
{
    if (monitor_pipe_fd == -1)
        return;

    fd_set rfds;
    struct timeval tv = {0, 0}; // non-blocking select
    FD_ZERO(&rfds);
    FD_SET(monitor_pipe_fd, &rfds);

    int ret = select(monitor_pipe_fd + 1, &rfds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(monitor_pipe_fd, &rfds))
    {
        char buffer[256];
        ssize_t n = read(monitor_pipe_fd, buffer, sizeof(buffer) - 1);
        if (n > 0)
        {
            buffer[n] = '\0';
            printf("[Monitor] %s", buffer);
        }
        else if (n == 0)
        {
            // EOF
            close(monitor_pipe_fd);
            monitor_pipe_fd = -1;
            monitor_running = 0;
            monitor_pid = -1;
            printf("[treasure_hub] Monitorul s-a inchis (EOF pipe)\n");
        }
    }
}

void handle_sigchld(int sig)
{
    int status;
    pid_t pid = waitpid(monitor_pid, &status, WNOHANG);
    if (pid == monitor_pid)
    {
        printf("[treasure_hub] Monitorul s-a inchis complet (SIGCHLD)\n");
        monitor_pid = -1;
        monitor_running = 0;
        if (monitor_pipe_fd != -1)
        {
            close(monitor_pipe_fd);
            monitor_pipe_fd = -1;
        }
    }
}

void setup_handlers()
{
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);
}

int main()
{
    setup_handlers();

    char buffer[256];

    while (1)
    {
        check_monitor_output();

        printf(">> ");
        fflush(stdout);

        if (!fgets(buffer, sizeof(buffer), stdin))
            break;
        buffer[strcspn(buffer, "\n")] = '\0';

        Command cmd = parse(buffer);
        switch (cmd)
        {
        case START_MONITOR:
        {
            start_monitor();
            break;
        }

        case LIST_HUNTS:
        {
            if (monitor_running)
                send_signal(monitor_pid, LIST_HUNTS);
            else
                printf("EROARE: Monitorul nu e pornit si nu mai primeste comenzi\n");
            break;
        }

        case LIST_TREASURES:
	  {
	    if (!monitor_running)
	      {
		printf("EROARE: Monitorul nu e pornit si nu mai primeste comenzi\n");
		break;
	      }

	    // Comanda cu argument: list_treasures <hunt_id>
	    char hunt_id[64];
	    printf("Hunt ID: ");
	    fflush(stdout);
	    if (!fgets(hunt_id, sizeof(hunt_id), stdin))
	      break;
	    hunt_id[strcspn(hunt_id, "\n")] = '\0';

	    // trimitem argumentul la monitor
	    send_command_with_args(hunt_id, LIST_TREASURES);
	    break;
	  }

	case VIEW_TREASURE:
	  {
	    if (!monitor_running)
	      {
		printf("EROARE: Monitorul nu e pornit si nu mai primeste comenzi\n");
		break;
	      }

	    char line[128];
	    char hunt_id[64], treasure_id[64];

	    printf("Hunt ID: ");
	    fflush(stdout);
	    if (!fgets(hunt_id, sizeof(hunt_id), stdin))
	      break;
	    hunt_id[strcspn(hunt_id, "\n")] = '\0';

	    printf("Treasure ID: ");
	    fflush(stdout);
	    if (!fgets(treasure_id, sizeof(treasure_id), stdin))
	      break;
	    treasure_id[strcspn(treasure_id, "\n")] = '\0';

	    snprintf(line, sizeof(line), "%s %s", hunt_id, treasure_id);

	    send_command_with_args(line, VIEW_TREASURE);
	    break;
	  }

        case STOP_MONITOR:
	  {
            if (monitor_running)
	      {
                send_signal(monitor_pid, STOP_MONITOR);
                if (monitor_pipe_fd != -1)
		  {
                    close(monitor_pipe_fd);
                    monitor_pipe_fd = -1;
		  }
                monitor_running = 0;
                monitor_pid = -1;
	      }
            else
	      printf("Monitorul nu este pornit\n");
            break;
	  }

        case EXIT:
	  {
            if (monitor_running)
	      send_signal(monitor_pid, STOP_MONITOR);
            exit(0);
            break;
	  }

        case CALCULATE_SCORE:
	  {
            char hunt_id[64];
            printf("Hunt ID: ");
            fflush(stdout);
            if (!fgets(hunt_id, sizeof(hunt_id), stdin))
	      break;
            hunt_id[strcspn(hunt_id, "\n")] = '\0';

            calculate_score(hunt_id);
            break;
	  }

        case UNKNOWN:
        default:
	  printf("\nComanda necunoscuta\n");
	  break;
        }
    }

    return 0;
    }

