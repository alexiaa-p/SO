#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "treasure_manager.h"

typedef struct
{
    char name[30];
    int score;
}user;

int find_name(user users[],int count,char name[])
{
    for(int i=0;i<count;i++)
    {
        if(strcmp(users[i].name,name)==0)
            return i;
    }

    return -1;
}

int main(int argc,char **argv)
{
    if(argc !=2)
    {
        printf("arguments\n");
        return -1;
    }

    char *hunt=argv[1];
    char file[200];
    snprintf(file,sizeof(file),"%s/%s",hunt,hunt);

    int fd=open(file,O_RDONLY);

    if(fd <0)
    {
        printf("error\n");
        return -1;

    }


    treasure t;
    user users[100];
    int count=0;

    while(read(fd,&t,sizeof(treasure))>0)
    {
        int i=find_name(users,count,t.username);

        if(i>=0)
        {
            users[i].score+=t.val;
        }else{
            if(count < 100)
            {
                strcpy(users[count].name, t.username);
                users[count].score = t.val;
                count++;
            }else
            {
                printf("Max users\n");
            }
        }
    }

    close(fd);

    printf("Scores for hunt %s:\n", hunt);
    for (int i=0;i<count; i++) {
        printf("%s: %d\n", users[i].name, users[i].score);
    }

    return 0;

}