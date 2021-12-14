#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <dirent.h>
#include <sys/socket.h>

#include "csapp.h"

// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10

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
    RIGHT
} MOVES;

typedef struct pData {
    int x; 
    int y;
    struct pData* next;
} playerData;

TILETYPE grid[GRIDSIZE][GRIDSIZE];

playerData playerPos;
int score;
int level;
int numTomatoes;
int numPlayers;

bool shouldExit = false;

TTF_Font* font;
int clientfd;

playerData* playerList = NULL;

int receive(int fd) {
    int n = 0;
    int bytesRecvd = 0;
    int num = 0;
    char* buf = (char*)&num;
    while (bytesRecvd < sizeof(int)) {
        n = read(fd, buf + bytesRecvd, (sizeof(int) - bytesRecvd));
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
            fprintf(stderr, "ErrorCode: %d\n", n);
            return;
        }
        bytesSent += n;
    }
}

void renderServer() {
    playerPos.x = receive(clientfd);
    playerPos.y = receive(clientfd);

    if (playerList != NULL) {
    	printf("ERROR: playerList was not NULL");
    	exit(0);
    }
    numPlayers = 0;
    numTomatoes = 0;
    for (int x = 0; x < GRIDSIZE; x++) {
        for (int y = 0; y < GRIDSIZE; y++) {
            grid[x][y] = receive(clientfd);
            if (grid[x][y] == TILE_PLAYER) {
                if (x == playerPos.x && y == playerPos.y) {
                    continue;
                }
                numPlayers++;
                playerData* playerN = malloc(sizeof(playerData) * 1);
                playerN->x = x;
                playerN->y = y;
                playerN->next = playerList;
                playerList = playerN;         
            } 
            else if (grid[x][y] == TILE_TOMATO) {
                numTomatoes++;
            }
        }
    }
    score = receive(clientfd);
    printf("Score: %d\n", score);
    level = receive(clientfd);
    printf("Level: %d\n", level);
}

void initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int rv = IMG_Init(IMG_INIT_PNG);
    if ((rv & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "Error initializing IMG: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "Error initializing TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

void moveTo(int x, int y, MOVES move)
{
    // Prevent falling off the grid
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return;

    // Sanity check: player can only move to 4 adjacent squares
    if (!(abs(playerPos.x - x) == 1 && abs(playerPos.y - y) == 0) &&
        !(abs(playerPos.x - x) == 0 && abs(playerPos.y - y) == 1)) {
        fprintf(stderr, "Invalid move attempted from (%d, %d) to (%d, %d)\n", playerPos.x, playerPos.y, x, y);
        return;
    }
    writer(clientfd, MOVE);
    writer(clientfd, move);
}

void handleKeyDown(SDL_KeyboardEvent* event)
{
    // ignore repeat events if key is held down
    if (event->repeat)
        return;

    if (event->keysym.scancode == SDL_SCANCODE_Q || event->keysym.scancode == SDL_SCANCODE_ESCAPE)
        shouldExit = true;

    if (event->keysym.scancode == SDL_SCANCODE_UP || event->keysym.scancode == SDL_SCANCODE_W)
        moveTo(playerPos.x, playerPos.y - 1, UP);

    if (event->keysym.scancode == SDL_SCANCODE_DOWN || event->keysym.scancode == SDL_SCANCODE_S)
        moveTo(playerPos.x, playerPos.y + 1, DOWN);

    if (event->keysym.scancode == SDL_SCANCODE_LEFT || event->keysym.scancode == SDL_SCANCODE_A)
        moveTo(playerPos.x - 1, playerPos.y, LEFT);

    if (event->keysym.scancode == SDL_SCANCODE_RIGHT || event->keysym.scancode == SDL_SCANCODE_D)
        moveTo(playerPos.x + 1, playerPos.y, RIGHT);
}

void processInputs()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				shouldExit = true;
				break;

            case SDL_KEYDOWN:
                handleKeyDown(&event.key);
				break;

			default:
				break;
		}
	}
    
}

void drawGrid(SDL_Renderer* renderer, SDL_Texture* grassTexture, SDL_Texture* tomatoTexture, SDL_Texture* mainPlayerTexture, SDL_Texture* nPlayerTexture)
{
    SDL_Rect dest;
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            dest.x = 64 * i;
            dest.y = 64 * j + HEADER_HEIGHT;
            SDL_Texture* texture = grid[i][j] == TILE_TOMATO ? tomatoTexture : grassTexture;
            SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, texture, NULL, &dest);
        }
    }

    dest.x = 64 * playerPos.x;
    dest.y = 64 * playerPos.y + HEADER_HEIGHT;
    SDL_QueryTexture(mainPlayerTexture, NULL, NULL, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, mainPlayerTexture, NULL, &dest);
    playerData* prev = playerList; 
    while (playerList != NULL) {
        prev = playerList;
        SDL_Rect playerN;
        playerN.x = 64 * playerList->x;
        playerN.y = 64 * playerList->y + HEADER_HEIGHT;
        SDL_QueryTexture(nPlayerTexture, NULL, NULL, &playerN.w, &playerN.h);
        SDL_RenderCopy(renderer, nPlayerTexture, NULL, &playerN);
        playerList = playerList->next;
        free(prev);
    }
}

void drawUI(SDL_Renderer* renderer)
{
    // largest score/level supported is 2147483647
    char scoreStr[18];
    char levelStr[18];
    sprintf(scoreStr, "Score: %d", score);
    sprintf(levelStr, "Level: %d", level);

    SDL_Color white = {255, 255, 255};
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreStr, white);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

    SDL_Surface* levelSurface = TTF_RenderText_Solid(font, levelStr, white);
    SDL_Texture* levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);

    SDL_Rect scoreDest;
    TTF_SizeText(font, scoreStr, &scoreDest.w, &scoreDest.h);
    scoreDest.x = 0;
    scoreDest.y = 0;

    SDL_Rect levelDest;
    TTF_SizeText(font, levelStr, &levelDest.w, &levelDest.h);
    levelDest.x = GRID_DRAW_WIDTH - levelDest.w;
    levelDest.y = 0;

    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDest);
    SDL_RenderCopy(renderer, levelTexture, NULL, &levelDest);

    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    SDL_FreeSurface(levelSurface);
    SDL_DestroyTexture(levelTexture);
}

int main(int argc, char* argv[])
{
    if(argc < 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    
    srand(time(NULL));
    level = 1;
    initSDL();

    font = TTF_OpenFont("resources/Burbank-Big-Condensed-Bold-Font.otf", HEADER_HEIGHT);
    if (font == NULL) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    playerPos.x = playerPos.y = GRIDSIZE / 2;
	
    SDL_Window* window = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (window == NULL) {
        fprintf(stderr, "Error creating app window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL)
	{
		fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
	}

    SDL_Texture *grassTexture = IMG_LoadTexture(renderer, "resources/grass.png");
    SDL_Texture *tomatoTexture = IMG_LoadTexture(renderer, "resources/tomato.png");
    SDL_Texture *nPlayerTexture = IMG_LoadTexture(renderer, "resources/player.png");
    SDL_Texture *mainPlayerTexture = IMG_LoadTexture(renderer, "resources/playerN.png");

    // server connection
    char* host, *port;

    host = argv[1];
    port = argv[2];

    clientfd = open_clientfd(host, port);
	if (clientfd < 0) {
		fprintf(stderr, "Connection error to server: %d\n", clientfd);
		exit(0);
	}
	
    // main game loop
    while (!shouldExit) {
        SDL_SetRenderDrawColor(renderer, 0, 105, 6, 255);
        SDL_RenderClear(renderer);

        processInputs();

		writer(clientfd, RENDER);
		renderServer();

        drawGrid(renderer, grassTexture, tomatoTexture, mainPlayerTexture, nPlayerTexture);
        drawUI(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // 16 ms delay to limit display to 60 fps
    }

    // clean up everything
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(tomatoTexture);
    SDL_DestroyTexture(nPlayerTexture);
    SDL_DestroyTexture(mainPlayerTexture);

    TTF_CloseFont(font);
    TTF_Quit();

    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(clientfd);
}
