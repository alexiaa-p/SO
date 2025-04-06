#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>


typedef struct GPS
{
  float latitude;
  float longitude;
}GPS;

typedef struct Treasure
{
  int ID;
  char username[40];
  GPS coordinates;
  char clue[50];
  int value;
}Treasure;



void add(int hunt_id, Treasure treasure)  //adauga un treasure in hunt-ul(directorul) specificat
{
  char str[20];
  sprintf(str, "%d", hunt_id); //calea relativa

  DIR *dir;
  dir = opendir(str);

  if(dir == NULL)  //daca nu exista directorul, il cream
    {
      int creare_dir;
      creare_dir = mkdir(str, S_IRWXU | S_IRWXG | S_IRWXO);

      if(creare_dir != 0)
	{
	  printf("\nEroare la creare dir");
	}
      else
	{
	  printf("\nDirector creat cu succes");
	}
    }

  char fisier[50];
  sprintf(fisier, "%s/treasure.dat", str);  //fisierul de comori

  int fis = open(fisier, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(fis < 0)
    {
      printf("Eroare la deschiderea fisierului de comori\n");
      return;
    }

  ssize_t scriere = write(fis, &treasure, sizeof(Treasure));  //scriem in fisier

  if(scriere < sizeof(Treasure))  //daca nu scriem nr de octeti corespunzatori, at inseamna ca nu e ok
    {
      printf("Eroare la scrierea in fisierul cu comori\n");
      close(fis);
      return;
    }

  close(fis);
}

/*
First print the hunt name, the (total) file size and last modification time of its treasure file(s),
then list the treasures.
 */

void list(int hunt_id)
{
  char str[20];
  sprintf(str, "%d", hunt_id); //calea relativa

  DIR *dir;
  dir = opendir(str);   //tratam si cazul in care am mai multe fisiere in director, nu doar unul

  if(dir == NULL)
    {
      printf("Nu exista hunt-ul cu id-ul specificat\n");
      exit(-1);
    }

  printf("Hunt ID: %d\n", hunt_id);
  printf("Treasures:\n");

  struct dirent *intrare;   //intrare director
  struct stat st;   //pt detaliile fisierelor

  while((intrare = readdir(dir)) != NULL)
    {
      if(!(strcmp(intrare->d_name, ".")) || (!strcmp(intrare->d_name, "..")))
	{
	  continue;
	}

      char fisier[50];
      sprintf(fisier, "%s/%s", str, intrare->d_name);

      if(stat(fisier, &st) < 0)
	{
	  printf("Eroare la stat\n");
	  exit(-1);
	}

      //detaliile fisierului
      printf("\nTreasure: %s", intrare->d_name);
      printf("\nDimensiune in bytes: %ld", st.st_size);
      printf("\nUltima modificare: %s", ctime(&st.st_mtime));

      //continutul treasure-ului
      Treasure t;

      int fis_open = open(fisier, O_RDONLY);

      if(fis_open < 0)
	{
	  printf("Eroare la deschidere fis\n");
	  exit(-1);
	}
      
      while (read(fis_open, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                printf("ID: %d\n", t.ID);
                printf("Username: %s\n", t.username);
                printf("Coordonate: %.4f, %.4f\n", t.coordinates.latitude, t.coordinates.longitude);
                printf("Clue: %s\n", t.clue);
                printf("Valoare: %d\n", t.value);
                printf("------------------\n");
            }
      close(fis_open);
    }
  closedir(dir);

}


int main(int argc, char **argv)
{                                   //+trb facut meniul pt comenzi: add, list etc
  /*if(argc != 3)
    {
      printf("Nu avem nr corespunzator de argumente\n");
      return 1;
      }*/

  Treasure t, p;
  t.ID=1;
  strcpy(t.username, "User1");
  t.coordinates.latitude = 44.4268;
  t.coordinates.longitude = 26.1025;
  strcpy(t.clue, "Comoara e langa o statuie");
  t.value = 100;

   p.ID=2;
  strcpy(p.username, "User2");
  p.coordinates.latitude = 44.4268;
  p.coordinates.longitude = 26.1025;
  strcpy(p.clue, "Comoara e langa barca");
  p.value = 100;

  int hunt_id = atoi(argv[1]);
  //add(hunt_id, t);
  // add(hunt_id, p);

  list(hunt_id);
  
  
  return 0;
}
