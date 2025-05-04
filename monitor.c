#include <stdio.h>
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



char g_hunt_name[100];   //parametrii pe care ii iau din fisierul txt unde scriu comenzile
int g_treasure_id = -1;


void list_hunts()   //hunt-urile + cate treasure-uri sunt in fiecare
{
  printf("(MONITOR )Afisam toate hunt-urile si numarul treasure-urilor din fiecare\n");

  DIR* dir = opendir(".");
    if (!dir) {
        perror("opendir");
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
                printf("Hunt: %s | Total Treasures: %d\n", entry->d_name, total);
            }
        }
    }

    closedir(dir);
}


void get_from_file()
{
  FILE *file = fopen("params.txt", "r");
    if (!file) {
        perror("fopen params.txt");
        return;
    }

    char line[300];
    while (fgets(line, sizeof(line), file)) {

      if(monitor_running == 0)
	{
	  printf("Monitorul e oprit si nu mai primeste comenzi\n");
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
	    list_treasure(g_hunt_name);
	  }
	}
	else if (strcmp(token, "view_treasure") == 0) {
	  token = strtok(NULL, " \n");
	  if (token) strcpy(g_hunt_name, token);

	  char *treasure_id_str = strtok(NULL, " \n");
	  if (treasure_id_str) g_treasure_id = atoi(treasure_id_str);

	  view_treasure(g_hunt_name, g_treasure_id);
	}
    }

    fclose(file);
}


void handle_list_hunts(int sig)   //sigusr1
{
  if(monitor_running)
    list_hunts();
  else
    printf("Monitorul e OPRIT si nu mai primeste comenzi\n");
}

void handle_list_treasures()   //un hunt, sigusr2
{
  if(monitor_running)
    list_treasure(g_hunt_name);
  else
     printf("Monitorul e OPRIT si nu mai primeste comenzi\n");
}

void handle_view_treasure()   //un treasure, sigint
{
  if(monitor_running)
    view_treasure(g_hunt_name, g_treasure_id);
  else
     printf("Monitorul e OPRIT si nu mai primeste comenzi\n");
}

void handle_stop_monitor()   //sigterm
{
  printf("[Monitor] Primim cerere de oprire... \n");
  usleep(2000000);
  monitor_running = 0;
  printf("Monitorul s-a oprit\n");
  exit(0);
}

void handle_sigchld(int sig) {
  int status;
  waitpid(monitor_pid, &status, 0);
  printf("[monitor] Monitorul s-a inchis complet (SIGCHLD)\n");
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
        perror("sigaction SIGUSR1");
        exit(1);
    }


    // handler pt SIGUSR2
    sa1.sa_handler = handle_list_treasures;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR2, &sa1, NULL) == -1) {
        perror("sigaction SIGUSR2");
        exit(1);
    }

     // handler pt SIGINT
    sa1.sa_handler = handle_view_treasure;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGINT, &sa1, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }

     // handler pt SIGTERM
    sa1.sa_handler = handle_stop_monitor;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGTERM, &sa1, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(1);
    }

    //SIGCHLD
    sa1.sa_handler = handle_sigchld;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sigaction(SIGCHLD, &sa1, NULL);
}

int main() {
 
    setup_handlers();
    get_from_file();
    return 0;
}
