#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_hunt.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s --add <hunt_id>\n", argv[0]);
        fprintf(stderr, "  %s --list <hunt_id>\n", argv[0]);
        fprintf(stderr, "  %s --view <hunt_id> <treasure_id>\n", argv[0]);
        fprintf(stderr, "  %s --remove_treasure <hunt_id> <treasure_id>\n", argv[0]);
        fprintf(stderr, "  %s --remove_hunt <hunt_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* optiune = argv[1];
    const char* hunt_id = argv[2];

    if (strcmp(optiune, "--add") == 0) {
        add_treasure(hunt_id);
    } else if (strcmp(optiune, "--list") == 0) {
        list_treasure(hunt_id);
    } else if (strcmp(optiune, "--view") == 0 && argc == 4) {
        int treasure_id = atoi(argv[3]);
        view_treasure(hunt_id, treasure_id);
    } else if (strcmp(optiune, "--remove_treasure") == 0 && argc == 4) {
        int treasure_id = atoi(argv[3]);
        remove_treasure(hunt_id, treasure_id);
    } else if (strcmp(optiune, "--remove_hunt") == 0) {
        remove_hunt(hunt_id);
    } else {
        fprintf(stderr, "Invalid option or missing arguments.\n");
    }

    return 0;
}
