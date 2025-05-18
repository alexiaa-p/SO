#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hub.h"

COMMAND parseCommand(const char *input) {
    if (strcmp(input, "start_monitor") == 0)
      return CMD_START_MONITOR;
    if (strcmp(input, "list_hunts") == 0)
      return CMD_LIST_HUNTS;
    if (strcmp(input, "list_treasures") == 0)
      return CMD_LIST_TREASURES;
    if (strcmp(input, "view_treasure") == 0)
      return CMD_VIEW_TREASURE;
    if (strcmp(input, "stop_monitor") == 0)
      return CMD_STOP_MONITOR;
    if (strcmp(input, "calculate_score") == 0)
      return CMD_CALCULATE_SCORE;
    if (strcmp(input, "exit") == 0)
      return CMD_EXIT;
    return CMD_INVALID;
}

void printMenu() {
    printf("Alege comanda\n");
    printf("- start_monitor\n");
    printf("- list_hunts\n");
    printf("- list_treasures\n");
    printf("- view_treasure\n");
    printf("- stop_monitor\n");
    printf("- calculate_score\n");
    printf("- exit\n");
}

int meniu() {
    char buffer[64];

    while (1) {
        printMenu();
        printf(">> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            perror("Eroare citire comanda");
            return -1;
        }

        buffer[strcspn(buffer, "\n")] = '\0'; 
        COMMAND cmd = parseCommand(buffer);

        switch (cmd) {
            case CMD_START_MONITOR:
                startMonitor();
                break;
            case CMD_LIST_HUNTS:
                listHunts();
                break;
            case CMD_LIST_TREASURES:
                listTreasures2();
                break;
            case CMD_VIEW_TREASURE:
                viewTreasure2();
                break;
            case CMD_STOP_MONITOR:
                stopMonitor();
                break;
            case CMD_CALCULATE_SCORE:
                calculateScore();
                break;
            case CMD_EXIT:
                if (closeProgram() == 0) return 0;
                break;
            default:
                system("clear");
                printf("Comanda invalida\n");
        }
    }
}

int main(void) {
    return meniu();
}

