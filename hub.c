#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hub.h"
#include "manager.h"

static int status = 0;
static pid_t monitorPID = -1;
static int isMonitorRunning = 0;

static int commPipe[2];
static char recvBuffer[MAX * MAX];
static int stdoutBackup = 0;

char *getUserInput() {
  char temp[MAX / 2];
  if (fgets(temp, sizeof(temp), stdin) == NULL) return NULL;

  char *result = malloc(strlen(temp) + 1);
  if (!result) {
    perror("malloc error");
    return NULL;
  }
  strcpy(result, temp);
  return result;
}

int countTreasures(const char *hunt) {
  char *path = dataFilepath((char *)hunt);
  if (!path) {
    printf("nu s-a gasit datapath");
    return -1;
  }

  int fd = open(path, O_RDONLY);
  free(path);

  if (fd == -1) {
    perror("Nu s-a gasit hunt-ul");
    return -1;
  }

  int fileSize = lseek(fd, 0, SEEK_END);
  if (fileSize == -1) {
    perror("lseek error");
    close(fd);
    return -1;
  }

  close(fd);
  return fileSize / sizeof(Treasure);
}

void displayHunts() {
  DIR *dir = opendir(".");
  if (!dir) {
    perror("opendir failed");
    return;
  }

  struct dirent *entry;
  struct stat statbuf;
  char fullPath[3 * MAX];

  while ((entry = readdir(dir))) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    snprintf(fullPath, sizeof(fullPath), "./%s", entry->d_name);
    if (stat(fullPath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode) &&
        strcmp(entry->d_name, ".git") != 0) {
      printf("Hunt: %s ", entry->d_name);
      printf("Nr treasures: %d\n", countTreasures(entry->d_name));
    }
  }

  closedir(dir);
  printf("\n");
}

void hubSignalHandler(int sig) {
  if (sig == SIGUSR1) {
    memset(recvBuffer, 0, sizeof(recvBuffer));
    if (read(commPipe[0], recvBuffer, sizeof(recvBuffer)) > 0)
      printf("%s\n", recvBuffer);
    else
      printf("err la signal handler\n");
  }
}

void monitorSignalHandler(int sig) {
  kill(getppid(), SIGCONT);

  switch (sig) {
    case SIGUSR1: {
      system("clear");
      printf("LIST HUNTS\n");

      stdoutBackup = dup(STDOUT_FILENO);
      dup2(commPipe[1], STDOUT_FILENO);
      displayHunts();
      dup2(stdoutBackup, STDOUT_FILENO);

      kill(getppid(), SIGUSR1);
      break;
    }
    case SIGUSR2: {
      system("clear");
      printf("LIST TREASURES\n");
      printf("HUNT:");

      char *hunt = getUserInput();
      if (hunt) {
        hunt[strcspn(hunt, "\n")] = '\0';

        char command[MAX];
        snprintf(command, sizeof(command), "./treasure_manager --list %s", hunt);
        free(hunt);

        stdoutBackup = dup(STDOUT_FILENO);
        dup2(commPipe[1], STDOUT_FILENO);
        system(command);
        dup2(stdoutBackup, STDOUT_FILENO);

        kill(getppid(), SIGUSR1);
      }
      break;
    }
    case SIGINT: {
      system("clear");
      printf("VIEW TREASURES\n");

      printf("HUNT:");
      char *hunt = getUserInput();
      if (!hunt) break;
      hunt[strcspn(hunt, "\n")] = '\0';

      printf("Treasure ID:");
      int tid = 0;
      if (scanf("%d", &tid) != 1) perror("Treasure ID err scanf");
      getchar();

      char command[MAX];
      snprintf(command, sizeof(command), "./treasure_manager --view %s %d", hunt, tid);
      free(hunt);

      stdoutBackup = dup(STDOUT_FILENO);
      dup2(commPipe[1], STDOUT_FILENO);
      system(command);
      dup2(stdoutBackup, STDOUT_FILENO);

      kill(getppid(), SIGUSR1);
      break;
    }
    case SIGTERM:
      system("clear");
      printf("monitorul s-a inchis\n");
      kill(getppid(), SIGCONT);
      close(commPipe[1]);
      _exit(0);
  }
}

int startMonitor() {
  if (isMonitorRunning) {
    system("clear");
    printf("Monitorul e deja pornit\n");
    return -1;
  }

  if (pipe(commPipe) == -1) {
    perror("pipe error");
    return -1;
  }

  struct sigaction sa = {.sa_handler = hubSignalHandler};
  sigaction(SIGUSR1, &sa, NULL);

  monitorPID = fork();
  if (monitorPID < 0) {
    perror("fork error");
    return -1;
  }

  if (monitorPID == 0) {
    struct sigaction childSA = {.sa_handler = monitorSignalHandler};
    sigaction(SIGUSR1, &childSA, NULL);
    sigaction(SIGUSR2, &childSA, NULL);
    sigaction(SIGINT, &childSA, NULL);
    sigaction(SIGTERM, &childSA, NULL);

    close(commPipe[0]);

    while (1) pause();
  } else {
    isMonitorRunning = 1;
    system("clear");
    printf("Monitorul a pornit\n");
    close(commPipe[1]);
  }

  return 0;
}

int listHunts() {
  if (!isMonitorRunning) {
    system("clear");
    printf("Monitorul nu e pornit\n");
    return -1;
  }

  if (kill(monitorPID, SIGUSR1) == -1) {
    perror("Signal netrimis");
    return -1;
  }

  pause();
  return 0;
}

int listTreasures2() {
  if (!isMonitorRunning) {
    system("clear");
    printf("Monitorul nu e pornit\n");
    return -1;
  }

  if (kill(monitorPID, SIGUSR2) == -1) {
    perror("Signal netrimis");
    return -1;
  }

  pause();
  return 0;
}

int viewTreasure2() {
  if (!isMonitorRunning) {
    system("clear");
    printf("Monitor not running.\n");
    return -1;
  }

  if (kill(monitorPID, SIGINT) == -1) {
    perror("Signal not sent :");
    return -1;
  }

  pause();
  return 0;
}

int stopMonitor() {
  if (!isMonitorRunning) {
    system("clear");
    printf("Monitorul nu e pornit\n");
    return -1;
  }

  if (kill(monitorPID, SIGTERM) == -1) {
    perror("Signal netrimis");
    return -1;
  }

  close(commPipe[0]);
  isMonitorRunning = 0;
  waitpid(monitorPID, &status, 0);
  printf("Monitorul s-a terminat cu statusul %d\n", WEXITSTATUS(status));
  return 0;
}

int closeProgram() {
  if (isMonitorRunning) {
    system("clear");
    printf("Mai intai opriti monitorul\n");
    return -1;
  }

  return 0;
}

int calculateScore() {
  system("clear");

  DIR *dir = opendir(".");
  if (!dir) {
    perror("opendir error");
    return -1;
  }

  struct dirent *entry;
  char dirs[MAX][MAX] = {0};
  int dirCount = 0;

  char pathBuf[3 * MAX];
  struct stat st;

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe creation failed");
    return -1;
  }

  while ((entry = readdir(dir))) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    snprintf(pathBuf, sizeof(pathBuf), "./%s", entry->d_name);
    if (stat(pathBuf, &st) == 0 && S_ISDIR(st.st_mode) &&
        strcmp(entry->d_name, ".git") != 0) {
      strcpy(dirs[dirCount++], entry->d_name);
    }
  }

  closedir(dir);

  for (int i = 0; i < dirCount; ++i) {
    if (fork() == 0) {
      close(pipefd[0]);

      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);

      char cmd[3 * MAX];
      snprintf(cmd, sizeof(cmd), "./calculate_score %s", dirs[i]);
      system(cmd);

      exit(0);
    }
  }

  for (int i = 0; i < dirCount; ++i) wait(NULL);

  char buf[MAX * MAX] = {0};
  read(pipefd[0], buf, sizeof(buf) - 1);
  printf("%s\n", buf);

  return 0;
}

