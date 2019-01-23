#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "crates.h"

Level *load_level(char **level_path)
{
    FILE *level_file = fopen(*level_path, "r"); 

    if (level_file == NULL) {
        return NULL; 
    }

    Level *level = malloc(sizeof(Level));
    level->crates = NULL;

    int x = 0;
    int y = 0;
    char ch;
    size_t crates_array_size;

    while (fscanf(level_file, "%c", &ch) != EOF) {  
        if (ch == 'x') {
            level->board[y][x] = CELL_SOLID;
        } else if (ch == '.') {
            level->board[y][x] = CELL_FREE;
        } else if (ch == 's') {
            level->player.x = x;
            level->player.y = y;
            level->board[y][x] = CELL_FREE;
        } else if (ch == 'f') {
            level->board[y][x] = CELL_FINISH;
        } else if (ch == 'c') {
            crates_array_size = ++level->n_crates * sizeof(Movable);
            level->crates = realloc(level->crates, crates_array_size);
            level->crates[level->n_crates - 1].x = x;
            level->crates[level->n_crates - 1].y = y;
            level->board[y][x] = CELL_FREE;
        } else if (ch == '\n') {
            y++;
            x = -1;  // will become 0 at the following increment
        }
        x++;
    }

    fclose(level_file);
    return level;
}

Movable *get_crate(Level *level, int x, int y)
{
    int i;
    for (i = 0; i < level->n_crates; i++) {
        if (level->crates[i].x == x && level->crates[i].y == y) {
            return &level->crates[i];
        }
    }
    return NULL;
}

void print_level(Level *level)
{
    char *str;
    int x, y;
    for (y = 0; y < LEVEL_SIZE; y++) {
        for (x = 0; x < LEVEL_SIZE; x++) {
            if (x == level->player.x && y == level->player.y) {
                str = "()";
            } else if (get_crate(level, x, y) != NULL) {
                str = "[]";
            } else if (level->board[y][x] == CELL_FINISH) {
                str = "<>";
            } else if (level->board[y][x] == CELL_FREE) {
                str = "..";
            } else if (level->board[y][x] == CELL_SOLID) {
                str = "##";
            }
            mvaddstr(y, x * 2, str);
        }
    }
    refresh();
}

int is_legal_move(Level *level, Movable *movable, int x, int y)
{
    int new_x = movable->x + x;
    int new_y = movable->y + y;

    if (level->board[new_y][new_x] != CELL_SOLID) {
        // If the new cell contains a crate,
        // return whether moving the crate in the same direction is legal
        // Works recursively if there are several crates next to each other
        Movable *crate = get_crate(level, new_x, new_y);
        if (crate != NULL) {
            return is_legal_move(level, crate, x, y);
        }

        return 1;
    }

    return 0;
}

void move_movable(Level *level, Movable *movable, int x, int y)
{
    int new_x = movable->x + x;
    int new_y = movable->y + y;

    if (is_legal_move(level, movable, x, y)) {
        // If there is a crate in the new cell, move it first
        Movable *crate = get_crate(level, new_x, new_y);
        if (crate != NULL) {
            move_movable(level, crate, x, y);  // move crate in same direction
        }

        movable->x = new_x;
        movable->y = new_y;
    }
}

int game_won(Level *level)
{
    int i, x, y;
    for (i = 0; i < level->n_crates; i++) {
        x = level->crates[i].x;
        y = level->crates[i].y;
        if (level->board[y][x] != CELL_FINISH) {
            return 0;
        }
    }
    return 1;
}

void play(Level *level)
{
    int ch;

    while (ch != 'q' && !game_won(level)) {
        print_level(level);

        ch = getch();
        if (ch == KEY_RIGHT || ch == 'l') {
            move_movable(level, &level->player, 1, 0);
        } else if (ch == KEY_DOWN || ch == 'j') {
            move_movable(level, &level->player, 0, 1);
        } else if (ch == KEY_LEFT || ch == 'h') {
            move_movable(level, &level->player, -1, 0);
        } else if (ch == KEY_UP || ch == 'k') {
            move_movable(level, &level->player, 0, -1);
        }
    }
}

void init_curses()
{
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, 1);
}

int main(int argc, char *argv[])
{
    if (argc >= 2) {
        char *level_path = argv[1];
        Level *level = load_level(&level_path);

        if (level == NULL) {
            printf("Error loading level \"%s\".\n", level_path);
            return 1;
        }

        init_curses();

        play(level);

        free(level->crates);
        free(level);

        return endwin();
    } else {
        puts("No level specified.");
        return 1;
    }
}
