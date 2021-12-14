#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#include "csapp.h"

#define GRID_SIZE 10
typedef struct sockaddr SA;

typedef enum {
    TILE_GRASS,
    TILE_TOMATO,
    TILE_PLAYER
} TILETYPE;

typedef enum {
    END,
    MOVE,
    RENDER
} CMD;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} MOVES;


TILETYPE grid[GRID_SIZE][GRID_SIZE] = {};

typedef struct pdata {
    int id;
    int x; 
    int y;
    struct pdata* next;
} playerData;

typedef struct gridData {
    int score;
    int level;
    int numTomatoes;
    int players;
    playerData* head;
} gridInfo;

int receive(int fd);
void writer(int fd, int num);
void init();
void createGrid();
int getPID();
void* runner(void* cptr);
int validateMove(MOVES* move, playerData* p, int* x, int* y);
void renderGrid(int x, int y, playerData* p);
int recvCmd(int clientfd, MOVES* move, CMD* cmd);
void getNewPos(int* x, int* y);
void writePos(int clientfd, playerData* p);
void writeGrid(int clientfd);
void writeGridData(int clientfd);


int pIDs = 0;
pthread_mutex_t pIDLock;
pthread_mutex_t gridLock;
gridInfo grid_data; 

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage ./server [PORT]\n");
        exit(0);
    }
    int listenfd, *connfdp;
    listenfd = open_listenfd(argv[1]);
	if (listenfd < 0) {
		fprintf(stderr, "Error with listening socket\n");
        exit(0);
	}
	srand(time(NULL));
	init();

    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    

    while (1) {
        connfdp = malloc(sizeof(int) * 1);
        *connfdp = accept(listenfd, (SA *) &clientaddr, &clientlen);
        pthread_t tid;
        pthread_create(&tid, NULL, runner, connfdp);
    }
}

int receive(int fd) {
    int n = 0;
    int bytesRecvd = 0;
    int num = 0;
    char* buf = (char*)&num;
    while (bytesRecvd < sizeof(int)) {
        n = read (fd, buf + bytesRecvd, (sizeof(int) - bytesRecvd));
        if (n < 0) {
            fprintf(stderr, "Error Code: %d\n", n);
            return -1;
        }
        bytesRecvd += n;
    }
    return *((int*) buf);
}

void writer(int fd, int num) {
    char* buf = (char*)&num;
    int bytesSent = 0;
    int n = 0;
    while (bytesSent < sizeof(int)) {
        n = write(fd, (buf + bytesSent), (sizeof(int) - bytesSent));
        if (n < 0) {
            fprintf(stderr, "Error Code: %d\n", n);
            return;
        }
        bytesSent += n;
    }
}

void init() {
    pthread_mutex_init(&gridLock, NULL);
    pthread_mutex_init(&pIDLock, NULL);
    grid_data.players = 0;
    grid_data.head = NULL;
    grid_data.level = 0;
    grid_data.score = 0;
    grid_data.numTomatoes = 0;
    createGrid();
}

void createGrid() {
    grid_data.numTomatoes = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == TILE_PLAYER) {
                continue;
            }
            double r = ((double) rand()) / ((double) RAND_MAX);
            if (r < 0.1) {
                grid[i][j] = TILE_TOMATO;
                grid_data.numTomatoes++;
            } else {
                grid[i][j] = TILE_GRASS;
            }
        }
    }
    while (grid_data.numTomatoes == 0) {
        createGrid();
    }
}

int getPID() {
    pthread_mutex_lock(&pIDLock);
    int id = pIDs++;
    pthread_mutex_unlock(&pIDLock);
    return id;
}

void* runner(void* cptr) {
    int clientfd = *((int*) cptr);
    pthread_detach(pthread_self());
    free(cptr);
    
    playerData* currPlayer = malloc(sizeof(playerData) * 1);
    currPlayer->id = getPID();
    
    pthread_mutex_lock(&gridLock);
    getNewPos(&currPlayer->x, &currPlayer->y);
    grid[currPlayer->x][currPlayer->y] = TILE_PLAYER;
    grid_data.players++;
    pthread_mutex_unlock(&gridLock);
    
    CMD cmd = RENDER;
	MOVES move = NONE;
    do {
        pthread_mutex_lock(&gridLock);
        if (cmd == MOVE) {
            int newX;
        	int newY;
            if (validateMove(&move, currPlayer, &newX, &newY) == 1) {
                renderGrid(newX, newY, currPlayer);
            }
        }
        if (cmd == RENDER) {
            writePos(clientfd, currPlayer);
            writeGrid(clientfd);
        	writeGridData(clientfd);
        }
        pthread_mutex_unlock(&gridLock);
    } while (recvCmd(clientfd, &move, &cmd) == 1);

    // Removing a player once the connection with that player is closed
    pthread_mutex_lock(&gridLock);
    grid_data.players--;
    grid[currPlayer->x][currPlayer->y] = TILE_GRASS;
    pthread_mutex_unlock(&gridLock);
    printf("Player ID %d has been disconnected.\n", currPlayer->id);
    free(currPlayer);
    return NULL;
}


int validateMove(MOVES* move, playerData* p, int* newX, int* newY) {
    switch (*move) {
    	case UP:
    		*newX = p->x;
    		*newY = p->y - 1;
    		break;
    	case DOWN:
    		*newX = p->x;
    		*newY = p->y + 1;
    		break;
    	case LEFT:
    		*newX = p->x - 1;
    		*newY = p->y;
    		break;
    	case RIGHT:
    	   	*newX = p->x + 1;
    		*newY = p->y;
    		break;
		default:
			printf("ERROR in validation\n");
			return -1;
    }
    
    if (*newX < 0 || *newX >= GRID_SIZE || *newY < 0 || *newY >= GRID_SIZE || grid[*newX][*newY] == TILE_PLAYER) {
    	return -1;
    }
    return 1;
} 


void renderGrid(int x, int y, playerData* p) {
	grid[p->x][p->y] = TILE_GRASS;
	p->x = x;
	p->y = y;
    if (grid[x][y] == TILE_TOMATO) {
        grid_data.score++;
        grid_data.numTomatoes--;
    }
    grid[x][y] = TILE_PLAYER;
    if (grid_data.numTomatoes == 0) {
        grid_data.level++;
        createGrid();
    }
}

int recvCmd(int clientfd, MOVES* move, CMD* cmd) {
    *cmd = receive(clientfd);
    switch (*cmd) {
        case MOVE: 
            *move = receive(clientfd);
            return 1;
        case END:
            return -1;
        case RENDER:
        	return 1;
        default:
            return -1;
    }
}

void getNewPos(int* x, int* y) {
    int initX = rand() % GRID_SIZE;
    int initY = rand() % GRID_SIZE;
    while (grid[initX][initY] != TILE_GRASS) {
        initX = rand() % GRID_SIZE;
        initY = rand() % GRID_SIZE;
    }
    *x = initX;
    *y = initY;
}

void writePos(int clientfd, playerData* p) {
    writer(clientfd, p->x);
    writer(clientfd, p->y);
}

void writeGrid(int clientfd) {
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            writer(clientfd, grid[row][col]);
        }
    }
}

void writeGridData(int clientfd) {
    writer(clientfd, grid_data.score);
    writer(clientfd, grid_data.level);
}
