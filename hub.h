#define MAX 100

typedef enum {
    CMD_INVALID = -1,
    CMD_START_MONITOR,
    CMD_LIST_HUNTS,
    CMD_LIST_TREASURES,
    CMD_VIEW_TREASURE,
    CMD_STOP_MONITOR,
    CMD_CALCULATE_SCORE,
    CMD_EXIT
} COMMAND;

int displayMenu();
int calculateScore();
int listHunts();
int startMonitor();
int listTreasures2();
int viewTreasure2();
int stopMonitor();
int closeProgram();
