#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Player{
    int posX;
    int posY;
} player, monster;

int main(int argc, char** argv){

    int boardX = atoi(argv[1]);
    int boardY = atoi(argv[2]);
    int plrX = atoi(argv[3]);
    int plrY = atoi(argv[4]);
    int goalX = atoi(argv[5]);
    int goalY = atoi(argv[6]);
    int monX = atoi(argv[7]);
    int monY = atoi(argv[8]);

    char** grid;
    int i,j;
    grid = (char**)malloc(boardX * sizeof(char**));
    for (i = 0; i < boardX; i++) {
        grid[i] = (char*)malloc(boardY * sizeof(char*));
    }
    for(i=0; i<boardX; i++){
        for(j=0; j<boardY; j++){
            grid[i][j] = (char)malloc(2 * sizeof(char));
        }
    }

    for(j=0; j<boardY; ++j){
        for(i=0; i<boardX; ++i){
            if(i == plrX && j == plrY){
                strcpy(&grid[i][j], "P");
                continue;
            }
            else if(i == monX && j == monY){
                strcpy(&grid[i][j], "M");
                continue;
            }
            else if(i == goalX && j == goalY){
                strcpy(&grid[i][j], "G");
                continue;
            }
            strcpy(&grid[i][j], ".");
        }
    }
    for(j=boardY-1; j>=0; --j){
        for(i=0; i<boardX; ++i){
            printf("%c\t", grid[i][j]);
        }
        printf("\n");
    }

    for (i = 0; i < boardX; i++) {
        free(grid[i]);
    }
    free(grid);
    
    

    return EXIT_SUCCESS;

}