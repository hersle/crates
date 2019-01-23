#define LEVEL_SIZE 10

int CELL_FREE = 0;
int CELL_SOLID = 1;
int CELL_FINISH = 2;

typedef struct {
    int x;
    int y;
} Movable;

typedef struct {
    int board[LEVEL_SIZE][LEVEL_SIZE] ;
    Movable player;
    Movable *crates;  // pointer to a array of level crates
    int n_crates;
} Level;
