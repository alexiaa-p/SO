#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

typedef enum {
    NECUNOSCUT,
    ADD,
    LIST,
    VIEW,
    REMOVE_TREASURE,
    REMOVE_HUNT
} Comanda;

typedef struct {
    float latitude;
    float longitude;
} GPS;

typedef struct {
    int ID;
    char username[40];
    GPS coordinates;
    char clue[50];
    int value;
} Treasure;

void log_treasure(int hunt_id, char *action, int treasure_id);
void add_tr(int hunt_id, Treasure treasure);
void list(int hunt_id);
void view(int hunt_id, int id);
void remove_treasure(int hunt_id, int id);
void remove_hunt(int hunt_id);
void logged_hunt_symlinks(void);

#endif 
