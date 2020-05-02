#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define H 15
#define W 20

int bx, by;
int vx, vy;
int px, py;
int radius, l, r;
int blocks[H][W] = {0};
int score;

void gotoxy(int x, int y) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle, pos);
}

void startup() {
    // 0 space, 1 ball, 2 bar, 3 blocks

    // ball
    bx = W / 2;
    by = H - 2;
    srand(time(NULL));
    vx = (rand() % 2 == 0) ? -1 : 1;
    vy = 0;
    blocks[by][bx] = 1;

    // bar
    radius = 3;
    px = W / 2;
    py = H - 1;
    l = px - radius;
    r = px + radius;
    int k;
    for (k = l; k <= r; k++) {
        blocks[py][k] = 2;
    }

    // initialize blocks
    int i, j;
    for (j = 0; j < h / 4; j++) {
        for (i = 0; i < w; i++) {
            blocks[j][i] = 3;
        }
    }
}

void show() {
    gotoxy(0, 0);
    int i, j;
    // top boundary
    printf("+");
    for (i = 0; i < w; i++) {
        printf("-");
    }
    printf("+\n");

    // everything inside
    for (j = 0; j < h; j++) {
        // left boundary
        printf("|");
        for (i = 0; i < w; i++) {
            if (blocks[j][i] == 0) {
                printf(" ");
            }
            else if (blocks[j][i] == 1) {
                printf("o");
            }
            else if (blocks[j][i] == 2) {
                printf("=");
            }
            else if (blocks[j][i] == 3) {
                printf("x");
            }
        }
        // right boundary
        printf("|\n");
    }

    printf("\nSCORE: %d\n", score);
}

void updateWithoutInput() {
    // ball catched
    if (by == h - 2) {
        if (bx >= l && bx <= r) {
            printf("\a");
            vy = -1;
            if (bx == l) {
                vx = -2;
            } else if (bx == r) {
                bx = 2;
            } else if (bx < px) {
                vx = -1;
            } else if (bx > px) {
                vx = 1;
            } else {
                vx = vx > 0 ? 1 : -1;
            }
        }
    }
    // ball is not catched
    if (by == h - 1) {
        printf("     FAILED :(     \n");
        system("pause");
        exit(0);
    }

    // hit block
    if (by < h / 4) {
        if (blocks[by - 1][bx] == 3) {
            blocks[by - 1][bx] = 0;
            vy = 1;
            score++;
            printf("\a");
        }
    }

    //update ball
    bx = bx + vx;
    if (bx > w - 1) bx = w - 1;
    if (bx < 0) bx = 0;
    by = by + vy;
    if (by == 0) vy = 1;
    if (bx == 0 || bx == w - 1) vx = -vx;
    blocks[by][bx] = 1;

    Sleep(100);
}

void updateWithInput() {
    char input;
    if(_kbhit()) {
        input = _getch();
        if (input == 'a') {
            blocks[py][r] = 0;
            px--;
            l = px - radius;
            r = px + radius;
            blocks[py][l] = 2;
        }
        if (input == 'd') {
        	blocks[py][l] = 0;
            px++;
            l = px - radius;
            r = px + radius;
            blocks[py][r] = 2;
        }
    }
}

int checkEnding() {
    int i, j;
    int end = 1;
    for (j = 0; j < h/4; j++) {
        for (i = 0; i < w; i++) {
            if (blocks[j][i] == 3) {
                end = 0;
                break;
            }
        }
    }
    return end;
}

void HideCursor() {
    CONSOLE_CURSOR_INFO ci = { 1, 0 };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

int main() {
    HideCursor();
    startup();
    while (1) {
        show();
        updateWithoutInput();
        updateWithInput();
        if (checkEnding()) {
            break;
        }
    }

    printf("\n\n\n\n\nDONE BALL PRACTISE\n\n\n\n\n");
    return 0;
}