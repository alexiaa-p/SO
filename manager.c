#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX 1024
#define treasureTXT 128
#define allocBUF 20
#define pathMAX 512

typedef struct {
  float x;
  float y;
} GPS;

typedef struct {
  int ID;
  char userName[treasureTXT];
  int value;
  char clue[treasureTXT];
  GPS gps;
} Treasure;

typedef enum {
  ADD,
  LIST,
  VIEW,
  REMOVE_TREASURE,
  REMOVE_HUNT
} Operation;


Operation parseOperation(char *argument) {
  if (strcmp(argument, "--add") == 0) {
    return ADD;
  } else if (strcmp(argument, "--list") == 0) {
    return LIST;
  } else if (strcmp(argument, "--view") == 0) {
    return VIEW;
  } else if (strcmp(argument, "--remove_treasure") == 0) {
    return REMOVE_TREASURE;
  } else if (strcmp(argument, "--remove_hunt") == 0) {
    return REMOVE_HUNT;
  }
  return 0;
}


void clearInputBuffer() {
  int c = 0;
  while ((c = getchar()) != '\n' && c != EOF);
}

//pt citire treasue
Treasure treasureRead(Treasure tres) {
  char buffer[MAX];

  while (1) {
    printf("ID: ");
    fgets(buffer, sizeof(buffer), stdin);
    if (sscanf(buffer, "%d", &tres.ID) == 1) break;
    printf("ID invalid\n");
  }

  printf("Username: ");
  fgets(tres.userName, treasureTXT, stdin);
  if (tres.userName[strlen(tres.userName) - 1] != '\n') {
    clearInputBuffer();
  } else {
    tres.userName[strcspn(tres.userName, "\n")] = '\0';
  }

  while (1) {
    printf("Value: ");
    fgets(buffer, sizeof(buffer), stdin);
    if (sscanf(buffer, "%d", &tres.value) == 1) break;
    printf("Value invalid\n");
  }

  printf("Clue: ");
  fgets(tres.clue, treasureTXT, stdin);
  if (tres.clue[strlen(tres.clue) - 1] != '\n') {
    clearInputBuffer();
  } else {
    tres.clue[strcspn(tres.clue, "\n")] = '\0';
  }

  while (1) {
    printf("X coordonata: ");
    fgets(buffer, sizeof(buffer), stdin);
    if (sscanf(buffer, "%f", &tres.gps.x) == 1) break;
    printf("Coordonate invalide\n");
  }

  while (1) {
    printf("Y coordonata: ");
    fgets(buffer, sizeof(buffer), stdin);
    if (sscanf(buffer, "%f", &tres.gps.y) == 1) break;
    printf("Coordonate invalide\n");
  }

  return tres;
}

//data.bin pt hunt
char *dataFilepath(char *hunt) {
  char buf[MAX] = {0};
  strncpy(buf, hunt, MAX - allocBUF);
  strcat(buf, "/data.bin");

  char *filepath = (char *)calloc(strlen(buf) + 1, sizeof(char));
  if (filepath == NULL) {
    perror("err la alloc\n");
    return NULL;
  }

  strcpy(filepath, buf);
  return filepath;
}

//returneaza fisierul log pt hunt
char *logFilepath(char *hunt) {
  char buf[MAX] = {0};
  strncpy(buf, hunt, MAX - allocBUF);
  strcat(buf, "/logged_hunt.txt");

  char *filepath = (char *)calloc(strlen(buf) + 1, sizeof(char));
  if (filepath == NULL) {
    perror("err la alloc\n");
    return NULL;
  }

  strcpy(filepath, buf);
  return filepath;
}


void treasurePrint(Treasure *x) {
  printf("ID: %d\n", x->ID);
  printf("Username: %s\n", x->userName);
  printf("Value: %d\n", x->value);
  printf("Clue: %s\n", x->clue);
  printf("Coordonate: %.2f - %.2f\n\n", x->gps.x, x->gps.y);
}

// returneaza calea unui symlink pt log
char *linkPath(char *hunt) {
  char buf[MAX] = {0};
  strcpy(buf, "logged_hunt-");
  strncat(buf, hunt, MAX - allocBUF);
  strcat(buf, ".txt");

  char *logLink = (char *)calloc(strlen(buf) + 1, sizeof(char));
  if (logLink == NULL) {
    perror("err la alloc\n");
    return NULL;
  }

  strncpy(logLink, buf, strlen(buf));
  return logLink;
}

//mesaj in log
int addLog(char *hunt, char *mess) {
  char *logPath = logFilepath(hunt);
  if (logPath == NULL) {
    printf("Log path nu a fost gasit\n");
    return -1;
  }

  int logFile = open(logPath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (logFile == -1) {
    perror("Log File nu s-a putut deschide");
    free(logPath);
    return -1;
  }

  char log[MAX] = {0};
  time_t NOW = time(0);
  struct tm *time_info = localtime(&NOW);
  strftime(log, sizeof(log), "%Y-%m-%d %H:%M:%S ", time_info);

  if (write(logFile, log, strlen(log)) == -1) {
    perror("err la write\n");
    close(logFile);
    free(logPath);
    return -1;
  }

  if (write(logFile, mess, strlen(mess)) == -1) {
    perror("err la write\n");
    close(logFile);
    free(logPath);
    return -1;
  }

  if (close(logFile) == -1) {
    perror("err la close\n");
    free(logPath);
    return -1;
  }

  struct stat buff;
  char *logLink = linkPath(hunt);

  // daca nu am symlink il face
  if (stat(logLink, &buff) != 0) {
    if (symlink(logPath, logLink) == -1) {
      perror("err symlink:");
      free(logPath);
      free(logLink);
      return -1;
    }
  }

  free(logPath);
  free(logLink);
  return 0;
}

// add treasure in hunt
int addTreasure(char *hunt) {
  mkdir(hunt, 0777); // Creeaza folderul hunt daca nu exista

  char *dataPath = dataFilepath(hunt);
  if (dataPath == NULL) {
    printf("datapath negasit\n");
    return -1;
  }

  int dataFile = open(dataPath, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (dataFile == -1) {
    perror("datafile nu s-a putut deschide\n");
    free(dataPath);
    return -1;
  }

  free(dataPath);

  Treasure tr = {0};
  tr = treasureRead(tr);

  off_t offset = 0;
  char checkName[treasureTXT] = {0};
  int bytesRead = 0;

  //daca am deja usernameul in treasure
  while (1) {
    if (lseek(dataFile, offset, SEEK_SET) == -1) {
      perror("err lseek\n");
      close(dataFile);
      return -1;
    }

    bytesRead = read(dataFile, &checkName, treasureTXT);
    if (bytesRead == 0) {
      break;
    }

    if (strcmp(tr.userName, checkName) == 0) {
      printf("Exista deja un treasure cu acest username\n\n");
      tr = treasureRead(tr);
      continue;
    }

    offset = offset + sizeof(Treasure);
  }

  if (lseek(dataFile, 0, SEEK_END) == -1) {
    perror("err lseek\n");
    close(dataFile);
    return -1;
  }

  //scrie treasure ul la sf de fisier
  if (write(dataFile, &tr, sizeof(Treasure)) == -1) {
    perror("err la write\n");
    close(dataFile);
    return -1;
  }

  close(dataFile);
  addLog(hunt, "Treasure adaugat cu succes\n");

  return 0;
}

//toate treasure urile din hunt
void listTreasures(char *hunt) {
  char *dataPath = dataFilepath(hunt);
  if (dataPath == NULL) {
    printf("dataPath negasit\n");
    return;
  }

  int dataFile = open(dataPath, O_RDONLY);
  if (dataFile == -1) {
    perror("dataFile nu s-a putut deschide\n");
    free(dataPath);
    return;
  }

  free(dataPath);

  Treasure tr;
  ssize_t bytesRead;

  printf("Lista Treasure:\n\n");

  while ((bytesRead = read(dataFile, &tr, sizeof(Treasure))) > 0) {
    treasurePrint(&tr);
  }

  if (bytesRead == -1) {
    perror("err la read\n");
  }

  close(dataFile);
}

//treasure ul cu un anumit ID
void viewTreasure(char *hunt, int id) {
  char *dataPath = dataFilepath(hunt);
  if (dataPath == NULL) {
    printf("dataPath negasit\n");
    return;
  }

  int dataFile = open(dataPath, O_RDONLY);
  if (dataFile == -1) {
    perror("dataFile nu s-a putut deschide\n");
    free(dataPath);
    return;
  }

  free(dataPath);

  Treasure tr;
  ssize_t bytesRead;
  int found = 0;

  while ((bytesRead = read(dataFile, &tr, sizeof(Treasure))) > 0) {
    if (tr.ID == id) {
      treasurePrint(&tr);
      found = 1;
      break;
    }
  }

  if (!found) {
    printf("Nu s-a gasit treasure cu ID-ul %d\n", id);
  }

  close(dataFile);
}

//sterge un treasure
void removeTreasure(char *hunt, int id) {
  char *dataPath = dataFilepath(hunt);
  if (dataPath == NULL) {
    printf("dataPath negasit\n");
    return;
  }

  int dataFile = open(dataPath, O_RDWR);
  if (dataFile == -1) {
    perror("datafile nu s-a putut deschide\n");
    free(dataPath);
    return;
  }

  Treasure tr;
  ssize_t bytesRead;
  off_t offset = 0;
  int found = 0;

  while ((bytesRead = read(dataFile, &tr, sizeof(Treasure))) > 0) {
    if (tr.ID == id) {
      found = 1;
      break;
    }
    offset += sizeof(Treasure);
  }

  if (!found) {
    printf("Nu s-a gasit treasure cu ID-ul %d\n", id);
    close(dataFile);
    free(dataPath);
    return;
  }

  //ultimul treasure din fisier
  off_t lastOffset = lseek(dataFile, -sizeof(Treasure), SEEK_END);
  if (lastOffset == -1) {
    perror("err lseek\n");
    close(dataFile);
    free(dataPath);
    return;
  }

  Treasure lastTreasure;
  if (read(dataFile, &lastTreasure, sizeof(Treasure)) == -1) {
    perror("err read\n");
    close(dataFile);
    free(dataPath);
    return;
  }

  //daca treasure ul sters nu e ultimul suprascriem cu ultimul
  if (offset != lastOffset) {
    if (lseek(dataFile, offset, SEEK_SET) == -1) {
      perror("err lseek\n");
      close(dataFile);
      free(dataPath);
      return;
    }
    if (write(dataFile, &lastTreasure, sizeof(Treasure)) == -1) {
      perror("err write\n");
      close(dataFile);
      free(dataPath);
      return;
    }
  }

  //trunchierea fisierului la dimensiunea fara ultimul treasure
  if (ftruncate(dataFile, lastOffset) == -1) {
    perror("err ftruncate\n");
  }

  close(dataFile);
  free(dataPath);
  addLog(hunt, "Treasure sters cu succes\n");
}

//sterge un hunt complet (folder + continut)
void removeHunt(char *hunt) {
  DIR *dir = opendir(hunt);
  if (!dir) {
    perror("nu se poate deschide directorul");
    return;
  }

  struct dirent *entry;
  char filepath[MAX];

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    snprintf(filepath, sizeof(filepath), "%s/%s", hunt, entry->d_name);
    if (remove(filepath) == -1) {
      perror("nu se poate sterge fisierul");
    }
  }
  closedir(dir);

  if (rmdir(hunt) == -1) {
    perror("nu se poate sterge directorul");
  } else {
    printf("Hunt-ul '%s' a fost sters complet.\n", hunt);
  }
}

