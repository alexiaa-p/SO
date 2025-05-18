#include <stdio.h>
#include <stdlib.h>
#include "manager.h"

int main(int argc, char **argv){
    if(argc < 2){
        return -1;
    }
    Operation op = parseOperation(argv[1]);

    switch(op){
        case ADD:
            if(argc < 3){
                printf("Lipsesc argumente\n");
                return -1;
            } 
            if(addTreasure(argv[2]) != 0){
                printf("Eroare la adaugarea comorii\n");
                return -1;
            }
            break;
        case LIST:
            if(argc < 3){
                printf("Lipsesc argumente\n");
                return -1;
            } 
            if(listTreasures(argv[2]) != 0){
                printf("Eroare la listarea comorilor\n");
                return -1;
            }
            break;
        case VIEW:
            if(argc < 4){
                printf("Lipsesc argumente\n");
                return -1;
            }
            if(viewTreasure(argv[2], argv[3]) != 0){
                printf("Eroare la afisarea comorii\n");
                return -1;
            }
            break;
        case REMOVE_TREASURE:
            if(argc < 4){
                printf("Lipsesc argumente\n");
                return -1;
            } 
            if(removeTreasure(argv[2], argv[3]) != 0){
                printf("Eroare la stergerea comorii\n");
                return -1;
            }
            break;
        case REMOVE_HUNT:
            if(argc < 3){
                printf("Lipsesc argumente\n");
                return -1;
            }
            if(removeHunt(argv[2]) != 0){
                printf("Eroare la stergerea hunt-ului\n");
                return -1;
            }
            break;
        default:
            printf("Optiune incorecta\n");
    }

    return 0;
}
