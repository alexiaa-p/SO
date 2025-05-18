#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "manager.h"

typedef struct {
  char username[MAX];
  int points;
} USER;

USER users[MAX];
int usersCount = 0;

void afiseazaScoruri(char *hunt) {
  printf("Scorurile pt hunt ul %s sunt:\n", hunt);
  for (int i = 0; i < usersCount; i++) {
    printf("%s : %d\n", users[i].username, users[i].points);
  }
}

int gasesteUser(char *name) {
  for (int i = 0; i < usersCount; i++) {
    if (strcmp(users[i].username, name) == 0) {
      return i;
    }
  }
  return -1;
}

int calculeazaScor(char *hunt) {
  char *pathFisier = dataFilepath(hunt);
  if (pathFisier == NULL) {
    printf("Datapath negaasit");
    return -1;
  }
  int fd = open(pathFisier, O_RDONLY);
  if (fd == -1) {
    perror("Hunt negasit");
    free(pathFisier);
    return -1;
  }

  free(pathFisier);
  Treasure t = {0};
  int idx = 0;

  while (1) {
    if (read(fd, &t, sizeof(t)) != sizeof(t)) {
      break;
    }

    idx = gasesteUser(t.userName);
    if (idx == -1) {
      strcpy(users[usersCount].username, t.userName);
      users[usersCount++].points = t.value;
    } else {
      users[idx].points += t.value;
    }
  }

  if (close(fd) == -1) {
    perror("err la fileclose");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Trebuie specificat hunt-ul\n");
    return -1;
  }

  calculeazaScor(argv[1]);
  afiseazaScoruri(argv[1]);
  return 0;
}
