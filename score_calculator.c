#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "treasure_hunt.h"

#define USERNAME 32
#define CLUE 128
#define FILENAME "treasure.dat"

/*typedef struct {
    int id;
    char username[USERNAME];
    float latitude;
    float longitude;
    char clue[CLUE];
    int value;
    } Treasure;*/

typedef struct User {
    char username[USERNAME];
    int score;
    struct User* next;
} User;

User* find_or_add_user(User** head, const char* username) {
    User* u = *head;
    while (u) {
        if (strcmp(u->username, username) == 0) 
            return u;
        u = u->next;
    }
   
    User* new_node = malloc(sizeof(User));
    strncpy(new_node->username, username, USERNAME);
    new_node->score = 0;
    new_node->next = *head;
    *head = new_node;
    return new_node;
}

void free_scores(User* head) {
    while (head) {
        User* tmp = head;
        head = head->next;
        free(tmp);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("eroare argumente");
        return 1;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", argv[1], FILENAME);

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("eroare fopen");
        return 1;
    }

    Treasure t;
    User* score_list = NULL;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        User* user = find_or_add_user(&score_list, t.username);
        user->score += t.value;
    }

    close(fd);

    User* curr = score_list;
    while (curr) {
        printf("%s: %d\n", curr->username, curr->score);
        curr = curr->next;
    }

    free_scores(score_list);
    return 0;
}
