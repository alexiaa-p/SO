
/*#include <stdio.h>  ///!!!!!!
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "treasure_hunt.h"

int fd_pipe_out = -1;

int monitor_running = 1;

char g_hunt_name[100];
int g_treasure_id = -1;

void list_hunts()
{
    dprintf(fd_pipe_out, "(MONITOR) Afisam toate hunt-urile si numarul treasure-urilor din fiecare\n");

    DIR* dir = opendir(".");
    if (!dir) {
        dprintf(fd_pipe_out, "[MONITOR] eroare la opendir\n");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            char treasure_path[300];
            sprintf(treasure_path, "%s/%s", entry->d_name, TREASURE);

            struct stat st;
            if (stat(treasure_path, &st) == 0) {
                int total = st.st_size / sizeof(Treasure);
                dprintf(fd_pipe_out, "Hunt: %s | Total Treasures: %d\n", entry->d_name, total);
            }
        }
    }

    closedir(dir);
}

void list_treasure_wrap(const char* hunt_id)
{
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(fd_pipe_out, STDOUT_FILENO);
    list_treasure(hunt_id);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

void view_treasure_wrap(const char* hunt_id, int treasure_id)
{
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(fd_pipe_out, STDOUT_FILENO);
    view_treasure(hunt_id, treasure_id);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

void calculate_score_wrap(const char* hunt_id)
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        dprintf(fd_pipe_out, "[monitor] eroare la pipe\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execl("./score_calculator", "score_calculator", hunt_id, (char*)NULL);
        perror("[monitor] exec score_calculator");
        exit(1);
    } else if (pid > 0) {
        close(pipefd[1]);
        char buffer[256];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            dprintf(fd_pipe_out, "%s", buffer);
        }
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
    } else {
        dprintf(fd_pipe_out, "[monitor] eroare la fork pentru score_calculator\n");
    }
}

void handle_list_hunts(int sig)
{
    if (monitor_running)
        list_hunts();
    else
        dprintf(fd_pipe_out, "Monitorul este oprit si nu mai primeste comenzi (SIGUSR1)\n");
}

void handle_list_treasures(int sig)
{
    if (monitor_running)
        list_treasure_wrap(g_hunt_name);
    else
        dprintf(fd_pipe_out, "Monitorul este oprit si nu mai primeste comenzi (SIGUSR2)\n");
}

void handle_view_treasure(int sig)
{
    if (monitor_running)
        view_treasure_wrap(g_hunt_name, g_treasure_id);
    else
        dprintf(fd_pipe_out, "Monitorul este oprit si nu mai primeste comenzi (SIGINT)\n");
}

void handle_stop_monitor(int sig)
{
    dprintf(fd_pipe_out, "[Monitor] Primim cerere de oprire (SIGTERM)...\n");
    monitor_running = 0;
    dprintf(fd_pipe_out, "Monitorul s-a oprit\n");
    exit(0);
}

void setup_handlers()
{
    struct sigaction sa;

    sa.sa_handler = handle_list_hunts;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_handler = handle_list_treasures;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    sa.sa_handler = handle_view_treasure;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = handle_stop_monitor;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
}

void process_command(char *line)
{
    if (!monitor_running) {
        dprintf(fd_pipe_out, "Monitorul este oprit, ignora comanda\n");
        return;
    }

    char *token = strtok(line, " \n");
    if (!token) return;

    if (strcmp(token, "list_hunts") == 0) {
        list_hunts();
    } else if (strcmp(token, "list_treasure") == 0) {
        token = strtok(NULL, " \n");
        if (token) {
            strcpy(g_hunt_name, token);
            list_treasure_wrap(g_hunt_name);
        } else {
            dprintf(fd_pipe_out, "Comanda list_treasure necesita un parametru\n");
        }
    } else if (strcmp(token, "view_treasure") == 0) {
        token = strtok(NULL, " \n");
        if (token) strcpy(g_hunt_name, token);

        char *treasure_id_str = strtok(NULL, " \n");
        if (treasure_id_str) g_treasure_id = atoi(treasure_id_str);
        else {
            dprintf(fd_pipe_out, "Comanda view_treasure necesita 2 parametri\n");
            return;
        }
        view_treasure_wrap(g_hunt_name, g_treasure_id);
    } else if (strcmp(token, "calculate_score") == 0) {
        token = strtok(NULL, " \n");
        if (token) {
            strcpy(g_hunt_name, token);
            calculate_score_wrap(g_hunt_name);
        } else {
            dprintf(fd_pipe_out, "Comanda calculate_score necesita un parametru\n");
        }
    } else if (strcmp(token, "start_monitor") == 0) {
        if (!monitor_running) {
            monitor_running = 1;
            dprintf(fd_pipe_out, "Monitorul a fost pornit (start_monitor)\n");
        } else {
            dprintf(fd_pipe_out, "Monitorul era deja pornit\n");
        }
    } else if (strcmp(token, "stop_monitor") == 0) {
        dprintf(fd_pipe_out, "Comanda stop_monitor primita, monitorul se opreste\n");
        monitor_running = 0;
        exit(0);
    } else {
        dprintf(fd_pipe_out, "Comanda necunoscuta: %s\n", token);
    }
}

int main(int argc, char **argv)
{
  /*if (argc != 2) {
        fprintf(stderr, "[monitor] lipseste file descriptor din argumente\n");
        exit(1);
	}*/

  //fd_pipe_out = atoi(argv[1]);

  fd_pipe_out = STDOUT_FILENO;
/* setup_handlers();

    dprintf(fd_pipe_out, "[monitor] Pornit si astept comenzi...\n");

   char line[256];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            // EOF, iesim
            break;
        }
        process_command(line);
    }

    dprintf(fd_pipe_out, "[monitor] stdin s-a inchis, monitorul se opreste\n");

    return 0;*/
}*/



/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>

#include "treasure_hunt.h"



int monitor_pid = -1;
int monitor_ending = 0; 
int monitor_ended = 0;
 int monitor_running = 1;


int fd_pipe_out = -1;    //argv[1] in linia de comanda

char g_hunt_name[100];   //parametrii pe care ii iau din fisierul txt unde scriu comenzile
int g_treasure_id = -1;


void list_hunts()   //hunt-urile + cate treasure-uri sunt in fiecare
{
  dprintf(fd_pipe_out, "(MONITOR )Afisam toate hunt-urile si numarul treasure-urilor din fiecare\n");

  DIR* dir = opendir(".");
    if (!dir) {
      //perror("opendir");
      dprintf(fd_pipe_out, "[MONITOR] eroare la opendir\n");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            char treasure_path[300];
	    sprintf(treasure_path, "%s/%s", entry->d_name, TREASURE);
	    

            struct stat st;
            if (stat(treasure_path, &st) == 0) {
                int total = st.st_size / sizeof(Treasure);
                dprintf(fd_pipe_out, "Hunt: %s | Total Treasures: %d\n", entry->d_name, total);
            }
        }
    }

    closedir(dir);
}


//WRAPPERE

void list_treasure_wrap(const char* hunt_id) {
    int saved_stdout = dup(STDOUT_FILENO);
    
    //stdout catre fd_pipe_out
    dup2(fd_pipe_out, STDOUT_FILENO);

    // functia normala (care folosește printf)
    list_treasure(hunt_id);

    // revine la stdout normal
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

void view_treasure_wrap(const char* hunt_id, int treasure_id) {
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(fd_pipe_out, STDOUT_FILENO);

    view_treasure(hunt_id, treasure_id);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

void calculate_score_wrap(const char* hunt_id) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        dprintf(fd_pipe_out, "[monitor] eroare la pipe\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // copilul
        close(pipefd[0]);          // închide capătul de citire
        dup2(pipefd[1], STDOUT_FILENO);  // stdout în pipe
        close(pipefd[1]);

        execl("./score_calculator", "score_calculator", hunt_id, (char*)NULL);
        perror("[monitor] exec score_calculator");
        exit(1);
    } else if (pid > 0) {
        // părinte
        close(pipefd[1]);          // închide capătul de scriere
        char buffer[256];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            // Trimite rezultatul către treasure_hub
            dprintf(fd_pipe_out, "%s", buffer);
        }
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
    } else {
        dprintf(fd_pipe_out, "[monitor] eroare la fork pentru score_calculator\n");
    }
}




////////////////////////////////////
void get_from_file()
{
  FILE *file = fopen("params.txt", "r");
    if (!file) {
      //perror("fopen params.txt");
      dprintf(fd_pipe_out, "fopen params.txt\n");
        return;
    }

    char line[300];
    while (fgets(line, sizeof(line), file)) {

      if(monitor_running == 0)
	{
	  dprintf(fd_pipe_out, "Monitorul e oprit si nu mai primeste comenzi\n");
	  break;
	}
        char *token = strtok(line, " \n");
        
        if (strcmp(token, "list_hunts") == 0) {
            list_hunts();  
        } 
	else if (strcmp(token, "list_treasure") == 0) {
	  token = strtok(NULL, " \n");
	  if (token) {
	    strcpy(g_hunt_name, token);
	    list_treasure_wrap(g_hunt_name);
	  }
	}
	else if (strcmp(token, "view_treasure") == 0) {
	  token = strtok(NULL, " \n");
	  if (token) strcpy(g_hunt_name, token);

	  char *treasure_id_str = strtok(NULL, " \n");
	  if (treasure_id_str) g_treasure_id = atoi(treasure_id_str);

	  view_treasure_wrap(g_hunt_name, g_treasure_id);
	}
	else if (strcmp(token, "calculate_score") == 0) {
	  token = strtok(NULL, " \n");
	  if (token) {
	    strcpy(g_hunt_name, token);
	    calculate_score_wrap(g_hunt_name);
	  }
	}

    }

    fclose(file);
}


void handle_list_hunts(int sig)   //sigusr1
{
  if(monitor_running)
    list_hunts();
  else
    dprintf(fd_pipe_out, "Monitorul e OPRIT si nu mai primeste comenzi\n");
}

void handle_list_treasures()   //un hunt, sigusr2
{
  if(monitor_running)
    list_treasure_wrap(g_hunt_name);
  else
    dprintf(fd_pipe_out, "Monitorul e OPRIT si nu mai primeste comenzi\n");
}

void handle_view_treasure()   //un treasure, sigint
{
  if(monitor_running)
    view_treasure_wrap(g_hunt_name, g_treasure_id);
  else
    dprintf(fd_pipe_out, "Monitorul e OPRIT si nu mai primeste comenzi\n");
}

void handle_stop_monitor()   //sigterm
{
  dprintf(fd_pipe_out, "[Monitor] Primim cerere de oprire... \n");
  usleep(2000000);
  monitor_running = 0;
  dprintf(fd_pipe_out, "Monitorul s-a oprit\n");
  exit(0);
}

void handle_sigchld(int sig) {
  int status;
  waitpid(monitor_pid, &status, 0);
  dprintf(fd_pipe_out, "[monitor] Monitorul s-a inchis complet (SIGCHLD)\n");
  monitor_ended = 1;
}


void setup_handlers()
{
  struct sigaction sa1;

    // handler pt SIGUSR1
    sa1.sa_handler = handle_list_hunts;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa1, NULL) == -1) {
      //perror("sigaction SIGUSR1");
      dprintf(fd_pipe_out, "sigaction SIUSR1\n");
        exit(1);
    }


    // handler pt SIGUSR2
    sa1.sa_handler = handle_list_treasures;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR2, &sa1, NULL) == -1) {
      // perror("sigaction SIGUSR2");
      dprintf(fd_pipe_out, "sigaction SIUSR2\n");
        exit(1);
    }

     // handler pt SIGINT
    sa1.sa_handler = handle_view_treasure;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGINT, &sa1, NULL) == -1) {
      // perror("sigaction SIGINT");
      dprintf(fd_pipe_out, "sigaction SIGINT\n");
        exit(1);
    }

     // handler pt SIGTERM
    sa1.sa_handler = handle_stop_monitor;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGTERM, &sa1, NULL) == -1) {
      // perror("sigaction SIGTERM");
      dprintf(fd_pipe_out, "sigaction SIGTERM\n");
        exit(1);
    }

    //SIGCHLD
    sa1.sa_handler = handle_sigchld;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sigaction(SIGCHLD, &sa1, NULL);
}

int main(int argc, char **argv) {

  if(argc != 2)
  {
    fprintf(stderr, "[monitor] lipseste file descriptor din argumente\n");
    exit(1);
  }

  fd_pipe_out = atoi(argv[1]);

  setup_handlers();

  // Citește o singură dată comenzile din params.txt, dacă vrei
  get_from_file();

  // Rămâne în viață și așteaptă semnale
  while (monitor_running) {
    pause();
  }

  // Dacă ajunge aici, înseamnă că s-a primit SIGTERM și monitorul se oprește
  dprintf(fd_pipe_out, "Monitorul se opreste efectiv\n");

  return 0;
  }*/

/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

int pipe_fd = -1; // descriptorul pipe-ului de scriere primit ca argument

// Exemplu de funcție pentru trimiterea mesajelor către treasure_hub
void send_message(const char *msg) {
    if (pipe_fd != -1) {
        dprintf(pipe_fd, "%s\n", msg);
    }
}

// Handler semnal SIGUSR1 -> list_hunts
void sigusr1_handler(int sig) {
    // Exemplu simplu: scrii mesajul în pipe
    send_message("Monitor: list_hunts command received");
    // aici pui logica reala de listare a hunt-urilor
}

// Handler semnal SIGUSR2 -> list_treasures
void sigusr2_handler(int sig) {
    send_message("Monitor: list_treasures command received");
    // aici pui logica reala de listare a treasure-urilor
}

// Handler semnal SIGINT -> view_treasure
void sigint_handler(int sig) {
    send_message("Monitor: view_treasure command received");
    // aici logica reala pentru afisarea detaliilor unui treasure
}

// Handler semnal SIGTERM -> oprire monitor
void sigterm_handler(int sig) {
    send_message("Monitor: received SIGTERM, exiting.");
    close(pipe_fd);
    exit(0);
}

// Funcție simplă pentru citirea comenzilor din fișier și trimiterea lor (dacă vrei)
void read_commands_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Eroare la deschiderea fisierului de comenzi");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // elimină newline-ul
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "list_hunts") == 0)
            sigusr1_handler(SIGUSR1);
        else if (strcmp(line, "list_treasures") == 0)
            sigusr2_handler(SIGUSR2);
        else if (strcmp(line, "view_treasure") == 0)
            sigint_handler(SIGINT);
        else if (strcmp(line, "stop_monitor") == 0)
            sigterm_handler(SIGTERM);
        else if (strlen(line) > 0)
            send_message("Monitor: Comanda necunoscuta in fisier.");
    }

    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pipe_write_fd>\n", argv[0]);
        return 1;
    }

    pipe_fd = atoi(argv[1]);

    // Setare handleri semnale
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    // Citire comenzi din fisier la start (daca exista)
    read_commands_from_file("params.txt");

    // Rămâi blocat, așteptând semnale
    while (1) {
        pause();
    }

    return 0;
}
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TREASURE "treasure.bin"

int monitor_running = 1;
char g_hunt_name[100];
int g_treasure_id = -1;

void list_hunts() {
    dprintf(STDOUT_FILENO, "(MONITOR) Afisam toate hunt-urile si numarul treasure-urilor din fiecare\n");

    DIR* dir = opendir(".");
    if (!dir) {
        dprintf(STDOUT_FILENO, "[MONITOR] eroare la opendir\n");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            char treasure_path[300];
            sprintf(treasure_path, "%s/%s", entry->d_name, TREASURE);

            struct stat st;
            if (stat(treasure_path, &st) == 0) {
                int total = st.st_size / sizeof(int); // simplificat
                dprintf(STDOUT_FILENO, "Hunt: %s | Total Treasures: %d\n", entry->d_name, total);
            }
        }
    }

    closedir(dir);
}

void list_treasure(const char* hunt) {
    dprintf(STDOUT_FILENO, "(MONITOR) Lista treasure-uri pentru hunt-ul '%s' (simulat)\n", hunt);
    char cale_treasure[256];
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);
    printf("deschidem %s\n", cale_treasure);

    int fd = open(cale_treasure, O_RDONLY);
    if (fd == -1) {
        perror("open treasure file");
        return;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("fstat");
        close(fd);
        return;
    }

    printf("Hunt ID: %s\n", hunt);
    printf("File Size: %ld bytes\n", file_stat.st_size);
    printf("Last Modified: %s", ctime(&file_stat.st_mtime));

    Treasure t;
    int treasure_count = 0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        treasure_count++;
        printf("\nTreasure ID: %d\n", t.treasure_id);
        printf("Username: %s\n", t.username);
        printf("GPS Coordinates: (%f, %f)\n", t.lati, t.longi);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
    }

    if (treasure_count == 0) {
        printf("\nNo treasures found in this hunt.\n");
    }

    close(fd);
}


void view_treasure(const char* hunt_id, int treasure_id) {
    dprintf(STDOUT_FILENO, "(MONITOR) Vizualizare treasure %d din hunt '%s' (simulat)\n", treasure_id, hunt);
    char cale_treasure[256];
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);

    int fd = open(cale_treasure, O_RDONLY);
    if (fd == -1) {
        perror("open treasure file");
        return;
    }

    Treasure t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            printf("\nTreasure ID: %d\n", t.treasure_id);
            printf("Username: %s\n", t.username);
            printf("GPS Coordinates: (%f, %f)\n", t.lati, t.longi);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Comoara cu treasure id %d nu s-a gasit.\n", treasure_id);
    }

    close(fd);
}


void calculate_score(const char* hunt) {
    dprintf(STDOUT_FILENO, "(MONITOR) Calculam scorul pentru hunt '%s' (simulat)\n", hunt);
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        dprintf(fd_pipe_out, "[monitor] eroare la pipe\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execl("./score_calculator", "score_calculator", hunt_id, (char*)NULL);
        perror("[monitor] exec score_calculator");
        exit(1);
    } else if (pid > 0) {
        close(pipefd[1]);
        char buffer[256];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            dprintf(fd_pipe_out, "%s", buffer);
        }
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
    } else {
        dprintf(fd_pipe_out, "[monitor] eroare la fork pentru score_calculator\n");
    }
}

void process_command(char *line) {
    if (!monitor_running) {
        dprintf(STDOUT_FILENO, "Monitorul este oprit, ignora comanda\n");
        return;
    }

    char *token = strtok(line, " \n");
    if (!token) return;

    if (strcmp(token, "list_hunts") == 0) {
        list_hunts();
    } else if (strcmp(token, "list_treasure") == 0) {
        token = strtok(NULL, " \n");
        if (token) {
            strcpy(g_hunt_name, token);
            list_treasure(g_hunt_name);
        } else {
            dprintf(STDOUT_FILENO, "Comanda list_treasure necesita un parametru\n");
        }
    } else if (strcmp(token, "view_treasure") == 0) {
        token = strtok(NULL, " \n");
        if (token) strcpy(g_hunt_name, token);
        else {
            dprintf(STDOUT_FILENO, "Comanda view_treasure necesita 2 parametri\n");
            return;
        }
        char *treasure_id_str = strtok(NULL, " \n");
        if (treasure_id_str) g_treasure_id = atoi(treasure_id_str);
        else {
            dprintf(STDOUT_FILENO, "Comanda view_treasure necesita 2 parametri\n");
            return;
        }
        view_treasure(g_hunt_name, g_treasure_id);
    } else if (strcmp(token, "calculate_score") == 0) {
        token = strtok(NULL, " \n");
        if (token) {
            strcpy(g_hunt_name, token);
            calculate_score(g_hunt_name);
        } else {
            dprintf(STDOUT_FILENO, "Comanda calculate_score necesita un parametru\n");
        }
    } else if (strcmp(token, "start_monitor") == 0) {
        if (!monitor_running) {
            monitor_running = 1;
            dprintf(STDOUT_FILENO, "Monitorul a fost pornit (start_monitor)\n");
        } else {
            dprintf(STDOUT_FILENO, "Monitorul era deja pornit\n");
        }
    } else if (strcmp(token, "stop_monitor") == 0) {
        dprintf(STDOUT_FILENO, "Monitorul se opreste.\n");
        monitor_running = 0;
        exit(0);
    } else {
        dprintf(STDOUT_FILENO, "Comanda necunoscuta: %s\n", token);
    }
}

int main() {
    dprintf(STDOUT_FILENO, "[monitor] Pornit si astept comenzi...\n");

    char line[256];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;  // EOF
        }
        process_command(line);
    }

    dprintf(STDOUT_FILENO, "[monitor] stdin s-a inchis, monitorul se opreste\n");
    return 0;
}

