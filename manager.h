#define treasureTXT 50
#define MAX 100
#define pathMAX 1000
#define allocBUF 20

typedef struct GPS {
  float x;
  float y;
} GPS;

typedef struct Treasure {
  char userName[treasureTXT];
  char clue[treasureTXT];
  GPS gps;
  int ID;
  int value;
} Treasure;

typedef enum { ADD, LIST, VIEW, REMOVE_TREASURE, REMOVE_HUNT } Operation;

void treasurePrint(Treasure *x);
Operation parseOperation(char *argument);
char *dataFilepath(char *hunt);
char *logFilepath(char *hunt);
Treasure treasureRead(Treasure x);
int addTreasure(char *hunt);
int addLog(char *hunt, char *message);
int listTreasures(char *hunt);
int viewTreasure(char *hunt, char *treasure);
int removeTreasure(char *hunt, char *treasure);
int removeHunt(char *hunt);