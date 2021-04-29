#include <curses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int board_x, board_y, board_size;
unsigned short color_amount = 0;

int* cache;
int* ecache;

int* board;
char* dialog = "Points: "; // Just in case you wanna translate it lol
unsigned long long score = 0;

int randpos() { return rand() % board_size; }

int rcindex(int row, int col) { return row * board_x + col; } // Converts row and column to index

int safeindex (int index) { if ( (index >= 0) && (index < board_size) ) { return index; } else { return -1; } }

int rand24() { return 1 + (rand() % 2); } // Should return either 1 or 2

bool isempty(int index) { if ( safeindex(index) == -1 ) { return FALSE; } else { return (board[index] == 0); }; }

void color(uint8_t r, uint8_t g, uint8_t b, bool black) {
    color_amount++;
    static unsigned int s = 1;
    const float mult = 3.9215;
    init_color(s + 7, r * mult, g * mult, b * mult);
    if (black) { init_pair(s, COLOR_BLACK, s + 7); }
    else       { init_pair(s, COLOR_WHITE, s + 7); }
    s++;
}

int findempty() { // Returns a random empty index, or -1 if there is none
    int i, j, k, swap;

    for (i = 0; i != 200; i++) { // Set the not equal to the amount of permutations
        j = randpos();
        k = randpos();
        swap = ecache[k];
        ecache[k] = ecache[j];
        ecache[j] = swap;
    }

    for (i = 0; i != board_size; i++) {
        if (isempty(ecache[i])) { return ecache[i]; }
    }
    return -1;
}

short resolve_keypress_to_direction(int a) {
    // Returns clockwise directions,
    // starting at up, ending at left.
    // Otherwise returns 0.

    switch(a) {

        // Up
        case 65:  // Arrow up
        case 107: // k
        case 119: // w
            return 1;

        // Right
        case 67:  // Arrow right
        case 100: // d
        case 108: // l
            return 2;

        // Down
        case 66:  // Arrow down
        case 106: // j
        case 115: // s
            return 3;

        // Left
        case 68:  // Arrow left
        case 97:  // a
        case 104: // h
            return 4;
    }
    return 0; // Error
}

bool is_loss() { // Resolves if the game is lost
    int i, j;

    // A shortcut, so that it doesn't
    // check everything if these are
    // still empty spaces
    for (i = 0; i != board_size; i++) {
        if (board[i] == 0) { return FALSE; }
    }

    // Look down
    for (i = 0; i != (board_y - 1); i++) {
        for (j = 0; j != board_x; j++) {
            if (board[rcindex(i, j)] == board[rcindex(i + 1, j)]) { return FALSE; }
        }
    }

    // Look up
    for (i = (board_y - 1); i != 0; i--) {
        for (j = 0; j != board_x; j++) {
            if (board[rcindex(i, j)] == board[rcindex(i - 1, j)]) { return FALSE; }
        }
    }

    /* These throw a warning while compiling,
     * for supposedly being out of bounds.
     * I haven't been able to get an error from this
     * so far, so it's probably a false positive, but
     * just in case, keep an eye out on this part.
     */

    // Look right
    for (i = 0; i != (board_x - 1); i++) {
        for (j = 0; j != board_y; j++) {
            if (board[rcindex(j, i)] == board[rcindex(j + 1, i)]) { return FALSE; }
        }
    }

    // Look left
    for (i = (board_x - 1); i != 0; i--) {
        for (j = 0; j != board_y; j++) {
            if (board[rcindex(j, i)] == board[rcindex(j - 1, i)]) { return FALSE; }
        }
    }

    return TRUE;
}

void draw_board() {
    unsigned long i, j, k;

    // Hardcoded constants
    const unsigned int offset_x = 0, offset_y = 2;

    // I recommend for these to be odd
    const unsigned long size_x = 7, size_y = 3;
    const unsigned int letter_x = size_x/2, letter_y = size_y/2;

    unsigned int off_x, off_y;

    char text[200];

    for (i = 0; i != (unsigned long)board_size; i++) {
        if ( cache[i] == board[i] ) { continue; } // No need to redraw unchanged

        // Calculates offset for given tile
        off_x = offset_x + ((i % board_x) * size_x);
        off_y = offset_y +((i / board_x) * size_y);

        attron(COLOR_PAIR((board[i] != 0) * ((board[i] - 1) % (color_amount - 1) + 2) + (board[i] == 0)));
        // This part draws the actual rectangle
        for (j = 0; j != size_y; j++) {
            for (k = 0; k != size_x; k++) {
                mvaddch(off_y +j, off_x + k, ' ');
            }
        }

        // This part does the text
        if ( board[i] == 0 ) { strcpy(text, "."); }
        else { sprintf(text, "%i", (int)(pow(2, board[i]))); }
        if ( strlen(text) > size_x ) { sprintf(text, "2^%i", board[i]); }

        mvprintw(off_y + letter_y, off_x + letter_x + (strlen(text) + 1) % 2 - strlen(text) / 2, text);

        attroff(COLOR_PAIR((board[i] != 0) * ((board[i] - 1) % (color_amount - 1) + 2) + (board[i] == 0)));


        cache[i] = board[i]; // Updates cache
    }
}

bool move_block(int r, int c, int dir) { // Moves block at given index in given direction
    int new, old = rcindex(r, c);
    bool work = 0;
    if (board[old] == 0) { return work; }
    int d[2] = {0, 0};
    switch (dir) {
        case 1: d[0] = -1; break;
        case 2: d[1] =  1; break;
        case 3: d[0] =  1; break;
        case 4: d[1] = -1; break;
    }

    while (isempty((new = rcindex(r + d[0], c + d[1])))) {
        if (c + d[1] > (board_x - 1) || c + d[1] < 0) { break; }
        old = rcindex(r, c);
        board[new] = board[old];
        board[old] = 0;

        r += d[0];
        c += d[1];
        work = 1;
    }

    if (c + d[1] > (board_x - 1) || c + d[1] < 0) { return work; }
    old = rcindex(r, c);
    if (board[old] == board[new]) {
        board[new]++;
        board[old] = 0;

        score += pow(2, board[new]);

        work = 1;
    }

    return work;
}

unsigned long game_logic(int dir) { // Does the game's logic. Returns if any move was made.
    unsigned long moves = 0;
    int r, c;

    switch(dir) {
        case 1:
            for (r = 0; r != board_y; r++) {
                for (c = 0; c != board_x; c++) {
                    moves += move_block(r, c, dir);
                }
            } break;
        case 2:
            for (r = 0; r != board_y; r++) {
                for (c = board_x - 1; c != -1; c--) {
                    moves += move_block(r, c, dir);
                }

            } break;
        case 3:
            for (r = board_y - 1; r != -1; r--) {
                for (c = 0; c != board_x; c++) {
                    moves += move_block(r, c, dir);
                }
            } break;
        case 4:
            for (r = 0; r != board_y; r++) {
                for (c = 0; c != board_x; c++) {
                    moves += move_block(r, c, dir);
                }
            } break;
    }

    return moves;
}

int main(int argc, char** argv) {
    int i, j, dir, newpieces;
    bool lost = FALSE;
    const unsigned int offset = strlen(dialog);
    int ch = 0;

    srand((unsigned)time(NULL)); // Random seed

    // Board size
    board_x = 4;
    board_y = 4;

    // Args
    if (argc >= 2) { board_x = atoi(argv[1]); }
    if (argc >= 3) { board_y = atoi(argv[2]); }

    board_size = board_x * board_y;
    if (!(board_size >= 3)) { printf("The board must be atleast 3 tiles big!\n"); exit(1); }

    // This should probably change
    newpieces = (board_size / 20) + 1;

    board = calloc(board_size, sizeof(int)); // This allocates memory
                                               // and initializes it to 0s

    cache = malloc(sizeof(int) * board_size);  // Draw cache
    for (i =0; i != board_size; i++) {
        cache[i] = -1;
    }

    ecache = malloc(sizeof(int) * board_size); // Findempty cache
    for (i = 0; i != board_size; i++) {
        ecache[i] = i;
    }

    board[findempty()] = rand24();
    board[findempty()] = rand24();

    initscr();
    clear();
    start_color();

    // Color Init
    color(50, 50, 50, FALSE); // First color is for 0 tiles
    color(255, 0, 0, FALSE);
    color(255, 128, 0, FALSE);
    color(255, 255, 0, TRUE);
    color(128, 255, 0, TRUE);
    color(0, 255, 0, TRUE);
    color(0, 255, 128, TRUE);
    color(0, 255, 255, TRUE);
    color(0, 128, 255, FALSE);
    color(0, 0, 255, FALSE);
    color(128, 0, 255, FALSE);
    color(255, 0, 255, FALSE);
    color(255, 0, 128, FALSE);

    noecho(); // Do not print keys out :eyes:
    curs_set(0);

    draw_board();

    mvprintw(0, 0, "%s0", dialog);

    refresh();

    while ( (ch = getch()) != 'q' ) {

        // Ignore unknown keystrokes
        if (!(dir = resolve_keypress_to_direction(ch))) { continue; }

        if (game_logic(dir) == 0) { continue; }

        // Adds new pieces

        for (i = 0; i != newpieces; i ++) {
            if ((j = findempty()) != -1) { board[j] = rand24(); }
            else { break; }
        }

        if (is_loss()) {
            lost = TRUE;
            break;
        }

        draw_board();

        mvprintw(0, offset, "%llu", score);

        refresh();
    }
    endwin(); // Ends it all so it doesn't cripple your terminal

    free(board);
    free(cache);
    free(ecache);

    if (lost) { printf("Good game!\nYour final score is %llu", score); }
}
