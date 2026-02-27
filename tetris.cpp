// tetris.cpp - Play Tetris in the terminal
// Minimal terminal Tetris using ncurses
// Compile: g++ tetris.cpp -std=c++17 -lncurses -O2 -o tetris

#include <ncurses.h>
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <random>

using namespace std;

const int BOARD_W = 10;
const int BOARD_H = 20;
const int PIECE_SIZE = 4;

using Piece = array<array<int, PIECE_SIZE>, PIECE_SIZE>;
vector<Piece> basePieces;

int board[BOARD_H][BOARD_W];

struct Game {
    int px = 3, py = 0;
    int cur = 0;
    int rot = 0;
    int next = 0;
    int score = 0;
    bool gameOver = false;
    mt19937 rng;
    uniform_int_distribution<int> dist;
    Game(): rng(random_device{}()), dist(0,6) {
        cur = dist(rng);
        next = dist(rng);
    }
} game;

vector<array<Piece,4>> allRotations;

Piece rotatePiece(const Piece &p) {
    Piece r{};
    for (int y=0;y<PIECE_SIZE;y++)
        for (int x=0;x<PIECE_SIZE;x++)
            r[x][PIECE_SIZE-1-y] = p[y][x];
    return r;
}

void initPieces() {
    // Define base 4x4 shapes
    // I
    Piece I = {{{0,0,0,0},
                {1,1,1,1},
                {0,0,0,0},
                {0,0,0,0}}};
    // O
    Piece O = {{{0,0,0,0},
                {0,1,1,0},
                {0,1,1,0},
                {0,0,0,0}}};
    // T
    Piece T = {{{0,0,0,0},
                {1,1,1,0},
                {0,1,0,0},
                {0,0,0,0}}};
    // S
    Piece S = {{{0,0,0,0},
                {0,1,1,0},
                {1,1,0,0},
                {0,0,0,0}}};
    // Z
    Piece Z = {{{0,0,0,0},
                {1,1,0,0},
                {0,1,1,0},
                {0,0,0,0}}};
    // J
    Piece J = {{{0,0,0,0},
                {1,1,1,0},
                {0,0,1,0},
                {0,0,0,0}}};
    // L
    Piece L = {{{0,0,0,0},
                {1,1,1,0},
                {1,0,0,0},
                {0,0,0,0}}};

    basePieces = {I,O,T,S,Z,J,L};

    allRotations.resize(basePieces.size());
    for (size_t i=0;i<basePieces.size();++i) {
        allRotations[i][0] = basePieces[i];
        for (int r=1;r<4;r++)
            allRotations[i][r] = rotatePiece(allRotations[i][r-1]);
    }
}

bool canPlace(int pieceIdx, int rot, int px, int py) {
    const Piece &p = allRotations[pieceIdx][rot];
    for (int y=0;y<PIECE_SIZE;y++) for (int x=0;x<PIECE_SIZE;x++) {
        if (!p[y][x]) continue;
        int bx = px + x;
        int by = py + y;
        if (bx < 0 || bx >= BOARD_W || by < 0 || by >= BOARD_H) return false;
        if (board[by][bx]) return false;
    }
    return true;
}

void placePiece(int pieceIdx, int rot, int px, int py, int val) {
    const Piece &p = allRotations[pieceIdx][rot];
    for (int y=0;y<PIECE_SIZE;y++) for (int x=0;x<PIECE_SIZE;x++) {
        if (!p[y][x]) continue;
        int bx = px + x;
        int by = py + y;
        if (by>=0 && by<BOARD_H && bx>=0 && bx<BOARD_W)
            board[by][bx] = val;
    }
}

void clearLines() {
    int cleared = 0;
    for (int y=BOARD_H-1;y>=0;y--) {
        bool full = true;
        for (int x=0;x<BOARD_W;x++) if (!board[y][x]) { full = false; break; }
        if (full) {
            cleared++;
            for (int yy=y; yy>0; yy--)
                for (int x=0;x<BOARD_W;x++)
                    board[yy][x] = board[yy-1][x];
            for (int x=0;x<BOARD_W;x++) board[0][x]=0;
            y++; // recheck same row
        }
    }
    if (cleared>0) game.score += (cleared*cleared)*100;
}

void spawnNext() {
    game.cur = game.next;
    game.next = game.dist(game.rng);
    game.px = 3;
    game.py = 0;
    game.rot = 0;
    if (!canPlace(game.cur, game.rot, game.px, game.py)) game.gameOver = true;
}

void drawWindow() {
    clear();
    int offsetX = 2, offsetY = 1;
    // Draw border
    for (int y=0;y<=BOARD_H+1;y++) {
        mvaddch(offsetY+y, offsetX-1, '|');
        mvaddch(offsetY+y, offsetX+BOARD_W, '|');
    }
    for (int x=-1;x<=BOARD_W;x++) {
        mvaddch(offsetY+BOARD_H, offsetX+x, '-');
    }
    // Board
    for (int y=0;y<BOARD_H;y++) for (int x=0;x<BOARD_W;x++) {
        if (board[y][x]) mvaddch(offsetY+y, offsetX+x, ACS_CKBOARD);
        else mvaddch(offsetY+y, offsetX+x, ' ');
    }
    // Current piece (ghost-free)
    const Piece &p = allRotations[game.cur][game.rot];
    for (int y=0;y<PIECE_SIZE;y++) for (int x=0;x<PIECE_SIZE;x++) {
        if (!p[y][x]) continue;
        int bx = game.px + x;
        int by = game.py + y;
        if (by>=0 && by<BOARD_H && bx>=0 && bx<BOARD_W)
            mvaddch(offsetY+by, offsetX+bx, ACS_CKBOARD);
    }
    // Next piece
    mvprintw(1, offsetX+BOARD_W+4, "Next:");
    const Piece &np = allRotations[game.next][0];
    for (int y=0;y<PIECE_SIZE;y++) for (int x=0;x<PIECE_SIZE;x++) {
        mvaddch(3+y, offsetX+BOARD_W+4 + x, np[y][x] ? ACS_CKBOARD : ' ');
    }
    mvprintw(9, offsetX+BOARD_W+4, "Score: %d", game.score);
    mvprintw(12, offsetX+BOARD_W+4, "Controls:");
    mvprintw(13, offsetX+BOARD_W+4, "Arrow keys - move");
    mvprintw(14, offsetX+BOARD_W+4, "z/x - rotate");
    mvprintw(15, offsetX+BOARD_W+4, "space - drop");
    mvprintw(16, offsetX+BOARD_W+4, "q - quit");
    if (game.gameOver) {
        mvprintw(BOARD_H/2, offsetX+BOARD_W/2-4, "GAME OVER");
        mvprintw(BOARD_H/2+1, offsetX+BOARD_W/2-8, "Press q to exit");
    }
    refresh();
}

int main() {
    initPieces();
    // init board
    for (int y=0;y<BOARD_H;y++) for (int x=0;x<BOARD_W;x++) board[y][x]=0;

    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    spawnNext();

    using clock = chrono::steady_clock;
    auto lastFall = clock::now();
    int fallMs = 500;
    bool running = true;

    while (running) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q' || ch=='Q') { running = false; break; }
            if (game.gameOver) { continue; }
            if (ch == KEY_LEFT) {
                if (canPlace(game.cur, game.rot, game.px-1, game.py)) game.px--;
            } else if (ch == KEY_RIGHT) {
                if (canPlace(game.cur, game.rot, game.px+1, game.py)) game.px++;
            } else if (ch == KEY_DOWN) {
                if (canPlace(game.cur, game.rot, game.px, game.py+1)) game.py++;
            } else if (ch == 'z' || ch=='Z') {
                int nr = (game.rot+3)%4;
                if (canPlace(game.cur, nr, game.px, game.py)) game.rot = nr;
            } else if (ch == 'x' || ch=='X') {
                int nr = (game.rot+1)%4;
                if (canPlace(game.cur, nr, game.px, game.py)) game.rot = nr;
            } else if (ch == ' ') {
                while (canPlace(game.cur, game.rot, game.px, game.py+1)) game.py++;
                placePiece(game.cur, game.rot, game.px, game.py, game.cur+1);
                clearLines();
                spawnNext();
            }
        }

        auto now = clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - lastFall).count() >= fallMs) {
            lastFall = now;
            if (!game.gameOver) {
                if (canPlace(game.cur, game.rot, game.px, game.py+1)) {
                    game.py++;
                } else {
                    // lock
                    placePiece(game.cur, game.rot, game.px, game.py, game.cur+1);
                    clearLines();
                    spawnNext();
                }
            }
        }

        drawWindow();
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    endwin();
    return 0;
}