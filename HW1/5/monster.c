#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Player{
    int posX;
    int posY;
} player, monster;

void printBoard(char** grid, int boardX, int boardY){
    int i,j;
    for(j=boardY-1; j>=0; --j){
        for(i=0; i<boardX; ++i){
            printf("%c\t", grid[i][j]);
        }
        printf("\n");
    }
}
char** updateGrid(char** grid, int goalX, int goalY, int boardX, int boardY){
    int i,j;
    
    for(j=0; j<boardY; ++j){
        for(i=0; i<boardX; ++i){
            if(i == player.posX && j == player.posY){
                strcpy(&grid[i][j], "P");
                continue;
            }
            else if(i == monster.posX && j == monster.posY){
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
    return grid;
}

void monsterMoves(int goalX, int goalY, int boardX, int boardY){
    int vertDist = player.posY - monster.posY;
    int horzDist = player.posX - monster.posX;
    if(!(player.posX == goalX && player.posY == goalY) 
            && !(player.posX == monster.posX && player.posY == monster.posY))
            {
                if(abs(vertDist)>abs(horzDist) || vertDist == horzDist){
                    if(vertDist>0 && monster.posY+1<boardY){
                        ++monster.posY; //Monster moves North
                        printf("monster moves N\n");
                    }
                    else if(vertDist<0 && monster.posY-1>=0){
                        --monster.posY; //Monster moves South
                        printf("monster moves S\n");
                    }
                }
                else{
                    if(horzDist>0 && monster.posX+1<boardX){
                        ++monster.posX; //Monster moves East
                        printf("monster moves E\n");
                    }
                    else if (horzDist<0 && monster.posX-1>=0){
                        --monster.posX; //Monster moves West
                        printf("monster moves W\n");
                    }
                }
                return;
            }
    return;
}

int main(int argc, char** argv){
    //Getting the parameters
    int boardX = atoi(argv[1]);
    int boardY = atoi(argv[2]);
    int plrX = atoi(argv[3]);
    int plrY = atoi(argv[4]);
    int goalX = atoi(argv[5]);
    int goalY = atoi(argv[6]);
    int monX = atoi(argv[7]);
    int monY = atoi(argv[8]);

    //Initializing the Player
    player.posX = plrX;
    player.posY = plrY;
    
    //Initializing the Monster
    monster.posX = monX;
    monster.posY = monY;

    //Creating the grid and allocating necessary space
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

    //Initializing the grid
    for(j=0; j<boardY; ++j){
        for(i=0; i<boardX; ++i){
            if(i == player.posX && j == player.posY){
                strcpy(&grid[i][j], "P");
                continue;
            }
            else if(i == monster.posX && j == monster.posY){
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
    printBoard(grid, boardX, boardY);

    size_t size = 3;
    char *move;
    move = (char*) malloc (size);

    //Playing the game
    while(!(player.posX == goalX && player.posY == goalY) 
            && !(player.posX == monster.posX && player.posY == monster.posY))
            {
                size_t num_bytes = getline(&move, &size, stdin);
                move[strcspn(move, "\n")] = '\0';
                
                if (strcasecmp(move,"N") == 0){
                    if(player.posY+1<boardY){
                        ++player.posY;
                    }
                    else{
                        printf("ENTER A NEW MOVE\n");
                        continue;
                    }
                }
                else if (strcasecmp(move,"S") == 0){
                    if(player.posY-1>=0){
                        --player.posY;
                    }
                    else{
                        printf("ENTER A NEW MOVE\n");
                        continue;
                    }
                }
                else if (strcasecmp(move,"E") == 0){
                    if(player.posX+1<boardX){
                        ++player.posX;
                    }
                    else{
                        printf("ENTER A NEW MOVE\n");
                        continue;
                    }
                    
                }
                else if (strcasecmp(move,"W") == 0){
                    if(player.posX-1>=0){
                        --player.posX;
                    }
                    else{
                        printf("ENTER A NEW MOVE\n");
                        continue;
                    }
                }
                else{
                    printf("ENTER A VALID MOVE\n");
                    continue;
                }

                monsterMoves(goalX,goalY,boardX,boardY);

                if(player.posX == goalX && player.posY == goalY){
                    printf("player wins!\n");
                }
                else if(player.posX == monster.posX && player.posY == monster.posY){
                    printf("monster wins!\n");
                }
                else{
                    grid = updateGrid(grid, goalX, goalY, boardX, boardY);
                    printBoard(grid,boardX,boardY);
                }
            }
            
    for (i = 0; i < boardX; i++) {
        free(grid[i]);
    }
    free(grid);
    
    

    return EXIT_SUCCESS;

}