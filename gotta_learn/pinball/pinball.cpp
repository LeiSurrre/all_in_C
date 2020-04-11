#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

int h, w;
int bx, by;
int vx, vy;
int px, py;
int radius, l, r;
//int ex, ey;
int blocks[100][100];
int score;

void gotoxy(int x, int y) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle, pos);
}

void startup() {
    h = 20;
    w = 20;
    bx = w / 2;
    by = h - 2;
    srand(time(NULL));
    vx = (rand() % 2 == 0) ? -1 : 1;
    vy = 0;
    radius = 3;
    px = w / 2;
    py = h - 1;
    l = px - radius;
    r = px + radius;
    //ex = ex = rand() % w;
    //ey = 0;

    // initialize blocks
    int i, j;
    for (j = 0; j < h / 4; j++) {
        for (i = 0; i < w; i++) {
            blocks[j][i] = 1;
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
            if (i >= l && i <= r && j == h - 1) {
                printf("+");
            }
            else if (i == bx && j == by) {
                printf("o");
            }
            else if (blocks[j][i] == 1) {
                printf("x");
                // else if (i == ex && j == ey) {
                //   printf("x");
            }
            else {
                printf(" ");
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

    // hit block old
    //if (bx == ex && by == ey) {
    //    score++;
    //    ex = rand()%w;
    //    vy = -vy;
    //}

    // hit block new
    if (by < h / 4) {
        if (blocks[by][bx] == 1) {
            blocks[by][bx] = 0;
            score++;
        }
    }

    //update ball
    bx = bx + vx;
    if (bx > w - 1) bx = w - 1;
    if (bx < 0) bx = 0;
    by = by + vy;
    if (by == 0) vy = 1;
    if (bx == 0 || bx == w - 1) vx = -vx;

    Sleep(100);
}

void updateWithInput() {
    char input;
    if(_kbhit()) {
        input = _getch();
        if (input == 'a') {
            px--;
            l = px - radius;
            r = px + radius;
        }
        if (input == 'd') {
            px++;
            l = px - radius;
            r = px + radius;
        }
    }
}

int checkEnding() {
    int i, j;
    int end = 1;
    for (j = 0; j < h/4; j++) {
        for (i = 0; i < w; i++) {
            if (blocks[j][i] == 1) {
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