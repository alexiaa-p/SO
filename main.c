#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "treasure_manager.h"

Comanda parse(int argc, char **argv) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "--add"))
            return ADD;
        if (!strcmp(argv[1], "--list"))
            return LIST;
        if (!strcmp(argv[1], "--view"))
            return VIEW;
        if (!strcmp(argv[1], "--remove"))
            if (argc == 4)
                return REMOVE_TREASURE;
            else
                return REMOVE_HUNT;
    }
    return NECUNOSCUT;
}

int main(int argc, char **argv) {
    Comanda cmd = parse(argc, argv);

    switch (cmd) {
        case ADD: {
            int hunt_id = atoi(argv[2]);
            Treasure t;
            
            printf("\nScrieti detaliile treasure-ului:\n");
            printf("ID: ");
            scanf("%d", &t.ID);

            printf("Username: ");
            scanf("%s", t.username); 

            printf("Latitudine: ");
            scanf("%f", &t.coordinates.latitude);

            printf("Longitudine: ");
            scanf("%f", &t.coordinates.longitude);

            printf("Indiciu: ");
            scanf("%s", t.clue);

            printf("Valoare: ");
            scanf("%d", &t.value);

            add_tr(hunt_id, t);
            break;
        }

        case LIST: {
            int hunt_id = atoi(argv[2]);
            printf("\nHunt-ul cu ID-ul %d este:\n", hunt_id);
            list(hunt_id);
            break;
        }

        case VIEW: {
            int hunt_id = atoi(argv[2]);
            int treasure_id = atoi(argv[3]);  
            view(hunt_id, treasure_id);
            break;
        }

        case REMOVE_TREASURE: {
            int hunt_id = atoi(argv[2]);
            int id = atoi(argv[3]);
            remove_treasure(hunt_id, id);
            break;
        }

        case REMOVE_HUNT: {
            int hunt_id = atoi(argv[2]);
            remove_hunt(hunt_id);
            break;
        }

        default:
            printf("Comanda necunoscuta\n");
    }

    logged_hunt_symlinks();

    return 0;
}
