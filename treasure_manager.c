#include "treasure_hunt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

void add_treasure(const char* hunt_id) {
    char cale_director[256];
    char cale_treasure[256];
    char cale_log[256];
    char cale_symlink[256];

    sprintf(cale_director, "%s", hunt_id);
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);
    sprintf(cale_log, "%s/%s", hunt_id, LOG);
    sprintf(cale_symlink, "logged_hunt-%s", hunt_id);

    if (mkdir(cale_director, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return;
    }

    Treasure t;
    printf("Treasure ID: ");
    scanf("%d", &t.treasure_id);
    getchar();

    printf("Username: ");
    fgets(t.username, USERNAME, stdin);
    t.username[strcspn(t.username, "\n")] = 0;

    printf("Latitude: ");
    scanf("%f", &t.lati);

    printf("Longitude: ");
    scanf("%f", &t.longi);
    getchar();

    printf("Clue: ");
    fgets(t.clue, CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;

    printf("Value: ");
    scanf("%d", &t.value);

    int fd = open(cale_treasure, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("eroare la deschidere");
        return;
    }
    write(fd, &t, sizeof(Treasure));
    close(fd);

    int log_fd = open(cale_log, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("eroare la deschidere");
        return;
    }

    time_t now = time(NULL);
    char log_entry[512];
    int len = sprintf(log_entry, "comoara cu treasure_id %d adaugata de %s la %s", t.treasure_id, t.username, ctime(&now));
    write(log_fd, log_entry, len);
    close(log_fd);

    unlink(cale_symlink);
    if (symlink(cale_log, cale_symlink) == -1) {
        perror("symlink");
    }
}

void list_treasure(const char* hunt_id) {
    char cale_treasure[256];
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);
    printf("deschidem %s\n", cale_treasure);

    int fd = open(cale_treasure, O_RDONLY);
    if (fd == -1) {
        perror("open treasure file");
        return;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("fstat");
        close(fd);
        return;
    }

    printf("Hunt ID: %s\n", hunt_id);
    printf("File Size: %ld bytes\n", file_stat.st_size);
    printf("Last Modified: %s", ctime(&file_stat.st_mtime));

    Treasure t;
    int treasure_count = 0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        treasure_count++;
        printf("\nTreasure ID: %d\n", t.treasure_id);
        printf("Username: %s\n", t.username);
        printf("GPS Coordinates: (%f, %f)\n", t.lati, t.longi);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
    }

    if (treasure_count == 0) {
        printf("\nNo treasures found in this hunt.\n");
    }

    close(fd);
}

void view_treasure(const char* hunt_id, int treasure_id) {
    char cale_treasure[256];
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);

    int fd = open(cale_treasure, O_RDONLY);
    if (fd == -1) {
        perror("open treasure file");
        return;
    }

    Treasure t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            printf("\nTreasure ID: %d\n", t.treasure_id);
            printf("Username: %s\n", t.username);
            printf("GPS Coordinates: (%f, %f)\n", t.lati, t.longi);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Comoara cu treasure id %d nu s-a gasit.\n", treasure_id);
    }

    close(fd);
}

void remove_treasure(const char* hunt_id, int treasure_id) {
    char cale_treasure[256];
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);
    char temp[256];
    sprintf(temp, "%s/%s.temp", hunt_id, TREASURE);

    int fd = open(cale_treasure, O_RDONLY);
    if (fd == -1) {
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    int temp_fd = open(temp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        perror("eroare la crearea fisierului temporar");
        close(fd);
        return;
    }

    Treasure t;
    int found = 0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            found = 1;
            continue;
        }
        write(temp_fd, &t, sizeof(Treasure));
    }

    close(fd);
    close(temp_fd);

    if (!found) {
        printf("Comoara cu treasure ID %d nu s-a gasit.\n", treasure_id);
        unlink(temp);
    } else {
        if (rename(temp, cale_treasure) == -1) {
            perror("eroare la inlocuirea fisierului");
        } else {
            printf("Comoara cu treasure ID %d a fost stearsa.\n", treasure_id);
        }
    }
}

void remove_hunt(const char* hunt_id) {
    char cale_director[256];
    sprintf(cale_director, "%s", hunt_id);

    char symlink_path[256];
    sprintf(symlink_path, "logged_hunt-%s", hunt_id);
    unlink(symlink_path);

    char cale_log[256];
    sprintf(cale_log, "%s/%s", hunt_id, LOG);
    unlink(cale_log);

    char cale_treasure[256];
    sprintf(cale_treasure, "%s/%s", hunt_id, TREASURE);
    unlink(cale_treasure);

    if (rmdir(cale_director) == -1) {
        perror("eroare la stergerea directorului");
    } else {
        printf("Hunt-ul %s a fost sters.\n", hunt_id);
    }
}
