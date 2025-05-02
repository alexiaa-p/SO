#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

#define USERNAME 32
#define CLUE 128
#define TREASURE "treasure.dat"
#define LOG "logged_hunt"

typedef struct {
    int treasure_id;
    char username[USERNAME];
    float lati;
    float longi;
    char clue[CLUE];
    int value;
} Treasure;

void add_treasure(const char* hunt_id);
void list_treasure(const char* hunt_id);
void view_treasure(const char* hunt_id, int treasure_id);
void remove_treasure(const char* hunt_id, int treasure_id);
void remove_hunt(const char* hunt_id);

#endif
