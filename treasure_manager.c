#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "treasure_manager.h"


void log_treasure(int hunt_id, char *action, int treasure_id)
{
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%d/logged_hunt.txt", hunt_id);

    FILE *log_file = fopen(log_path, "a");
    if (log_file == NULL) {
        perror("Eroare la deschiderea fisierului de log");
        return;
    }

    fprintf(log_file, "Actiune: %s | Treasure ID: %d\n", action, treasure_id);
    fclose(log_file);
}

void add_tr(int hunt_id, Treasure treasure) {
    char str[32];
    snprintf(str, sizeof(str), "%d", hunt_id);

    DIR *dir = opendir(str);
    if (dir == NULL) {
        if (mkdir(str, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
            perror("Eroare la creare director");
            return;
        } else {
            printf("Director creat cu succes\n");
        }
    } else {
        closedir(dir);
    }

    char fisier[300];
    snprintf(fisier, sizeof(fisier), "%s/treasure.dat", str);

    int fis = open(fisier, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fis < 0) {
        perror("Eroare la deschiderea fisierului de comori");
        return;
    }

    if (write(fis, &treasure, sizeof(Treasure)) < sizeof(Treasure)) {
        perror("Eroare la scrierea in fisierul cu comori");
        close(fis);
        return;
    }

    close(fis);
    log_treasure(hunt_id, "Adaugare comoara", treasure.ID);
}

void list(int hunt_id) {
    char fisier[64];
    snprintf(fisier, sizeof(fisier), "%d/treasure.dat", hunt_id);

    int fis_open = open(fisier, O_RDONLY);
    if (fis_open < 0) {
        printf("Nu exista hunt-ul cu ID-ul %d\n", hunt_id);
        return;
    }

    printf("Hunt ID: %d\nTreasures:\n", hunt_id);

    Treasure t;

    while (read(fis_open, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %d\n", t.ID);
        printf("Username: %s\n", t.username);
        printf("Coordonate: %.4f, %.4f\n", t.coordinates.latitude, t.coordinates.longitude);
        printf("Clue: %s\n", t.clue);
        printf("Valoare: %d\n", t.value);
        printf("------------------\n");
    }

    close(fis_open);

    log_treasure(hunt_id, "Listare hunt", -1);
}


void view(int hunt_id, int id) {
    char str[32];
    snprintf(str, sizeof(str), "%d", hunt_id);

    DIR *dir = opendir(str);
    if (dir == NULL) {
        printf("Nu exista hunt-ul cu id-ul specificat\n");
        return;
    }

    struct dirent *intrare;

    while ((intrare = readdir(dir)) != NULL) {
        if (strcmp(intrare->d_name, ".") == 0 || strcmp(intrare->d_name, "..") == 0)
            continue;

        char fisier[300];
        snprintf(fisier, sizeof(fisier), "%s/%s", str, intrare->d_name);

        Treasure t;
        int fis_open = open(fisier, O_RDONLY);
        if (fis_open < 0) {
            perror("Eroare la deschidere fisier");
            continue;
        }

        while (read(fis_open, &t, sizeof(Treasure)) == sizeof(Treasure)) {
            if (t.ID == id) {
                printf("ID: %d\n", t.ID);
                printf("Username: %s\n", t.username);
                printf("Coordonate: %.4f, %.4f\n", t.coordinates.latitude, t.coordinates.longitude);
                printf("Clue: %s\n", t.clue);
                printf("Valoare: %d\n", t.value);
                printf("------------------\n");
                break;
            }
        }

        close(fis_open);
    }

    closedir(dir);
}



void remove_treasure(int hunt_id, int id) {
    char str[20];
    sprintf(str, "%d", hunt_id);

    DIR *dir;
    dir = opendir(str);
    struct dirent *intrare;

    if (dir == NULL) {
        printf("Nu exista hunt-ul cu id-ul specificat\n");
        exit(-1);
    }

    while ((intrare = readdir(dir)) != NULL) {
        if (!(strcmp(intrare->d_name, ".")) || (!strcmp(intrare->d_name, ".."))) {
            continue;
        }

        char fisier[300];
        sprintf(fisier, "%s/%s", str, intrare->d_name);

        int fis_open = open(fisier, O_RDWR);
        if (fis_open < 0) {
            perror("eroare la deschiderea fisierului");
            exit(-1);
        }

        Treasure t;
        off_t pos = 0;
        off_t read_pos = 0;
        off_t write_pos = 0;
        int found = 0;

        while (read(fis_open, &t, sizeof(Treasure)) == sizeof(Treasure)) {
            if (t.ID == id) {
                found = 1;
                break;
            }
            pos += sizeof(Treasure);
        }

        if (!found) {
            printf("Treasure-ul cu ID %d nu a fost gasit\n", id);
            close(fis_open);
            continue;
        }

        read_pos = pos + sizeof(Treasure);
        write_pos = pos;

        while (read(fis_open, &t, sizeof(Treasure)) == sizeof(Treasure)) {
            pwrite(fis_open, &t, sizeof(Treasure), write_pos);
            read_pos += sizeof(Treasure);
            write_pos += sizeof(Treasure);
        }

        if (ftruncate(fis_open, write_pos) != 0) {
            perror("eroare la trunchiere fisier");
        }

        close(fis_open);
        printf("Treasure-ul cu ID %d a fost sters\n", id);
        log_treasure(hunt_id, "Stergere comoara", id);
    }

    closedir(dir);
}

void remove_hunt(int hunt_id)
{
    char fis[256];
    snprintf(fis, sizeof(fis), "%d", hunt_id);

    DIR *dir = opendir(fis);
    if (!dir) {
        printf("Hunt-ul cu ID %d nu exista\n", hunt_id);
        return;
    }

    struct dirent *intrare;
    char cale[512];

    while ((intrare = readdir(dir)) != NULL) {
        if (!strcmp(intrare->d_name, ".") || !strcmp(intrare->d_name, ".."))
            continue;

        snprintf(cale, sizeof(cale), "%s/%s", fis, intrare->d_name);

        struct stat st;
        if (stat(cale, &st) == 0) {
                if (remove(cale) != 0)
                    perror("Eroare la stergerea fisierului");
            
        }
    }
    
    closedir(dir);

    if (rmdir(fis) != 0) {
        perror("Eroare la stergerea directorului hunt");
    } else {
        printf("Hunt-ul cu ID %d a fost sters\n", hunt_id);
        log_treasure(hunt_id, "Stergere hunt", -1);
    }
}

void logged_hunt_symlinks() {
    DIR *root = opendir(".");
    if (!root) {
        perror("Eroare la deschiderea directorului curent");
        return;
    }

    struct dirent *intrare;

    while ((intrare = readdir(root)) != NULL) {
        if (intrare->d_type != DT_DIR)
            continue;

        if (strcmp(intrare->d_name, ".") == 0 || strcmp(intrare->d_name, "..") == 0)
            continue;

        int id = atoi(intrare->d_name);
        if (id == 0 && strcmp(intrare->d_name, "0") != 0)
            continue;

        char source[300];
        snprintf(source, sizeof(source), "%s/logged_hunt.txt", intrare->d_name);

        char link_name[64];
        snprintf(link_name, sizeof(link_name), "logged_hunt-%d", id);

        if (symlink(source, link_name) == 0) {
            printf("Link creat: %s -> %s\n", link_name, source);
        } else {
            switch (errno) {
	      case EEXIST:
                    printf("Link-ul %s deja exista\n", link_name);
                    break;
                case ENOENT:
                    printf("Fișierul sursă nu există: %s\n", source);
                    break;
                default:
                    perror("Eroare la creare symlink");
                    break;
            }
        }
    }

    closedir(root);
}



