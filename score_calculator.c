#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_USERS 100
#define MAX_NAME 64

typedef struct {
    char treasure_id[32];
    char username[64];
    float latitude, longitude;
    char clue[256];
    int value;
} Treasure;

typedef struct {
    char name[MAX_NAME];
    int score;
} UserScore;

UserScore users[MAX_USERS];
int user_count = 0;

void add_score(const char *username, int value) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].name, username) == 0) {
            users[i].score += value;
            return;
        }
    }
    //utilizator nou
    strncpy(users[user_count].name, username, MAX_NAME - 1);
    users[user_count].score = value;
    user_count++;
}

void process_file(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        perror("fopen");
        return;
    }

    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, f) == 1) {
        add_score(t.username, t.value);
    }

    fclose(f);
}

void process_directory(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char fullpath[512];

    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);
            process_file(fullpath);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_directory>\n", argv[0]);
        return 1;
    }

    process_directory(argv[1]);

    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", users[i].name, users[i].score);
    }

    return 0;
}
