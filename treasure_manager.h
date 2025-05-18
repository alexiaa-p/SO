#include <stdio.h>

typedef struct treasure 
{
    char id[16];
    char username[32];
    double latitude;
    double longitude;
    char clue[64];
    int val;

}treasure;

void add(char *hunt_id);
void list(char *hunt_id);
void view(char *hunt_id, char *treasure_id);
void remove_treasure(char *hunt_id, char *treasure_id);
void remove_hunt(char *hunt_id);
