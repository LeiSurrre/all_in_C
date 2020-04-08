#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

int px, py;
int h, w;
int bx, by;
int ex, ey;
int score;

void startup() {
    h = 20;
    w = 40;
    // jet
    px = w/2;
    py = h-1;
    // bullet
    by = -1;
    bx = -1;
    // target
    ey = 0;
    srand(time(NULL));
    ex = rand() % (w - 2) + 1;
    score = 0;
}

void gotoxy(int x, int y) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle, pos);
}

void show() {
    gotoxy(0, 0);
    //system("cls");
    int i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if (i == px && j == py) {
                printf("*");
            } else if (i == bx && j == by) {
                printf("|");
            } else if (i == ex && j == ey) {
                printf("x");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("SCORE: %d\n", score);
}

void updateWithoutInput() {
    if (by > -1) {
        by--;
    } else {
        // reset bullet
        bx = -1;
        by = -1;
    }

    if (bx == ex && by == ey) {
        score++;

        // reset target
        ey = 0;
        ex = rand() % (w - 2) + 1;

        // reset bullet
        bx = -1;
        by = -1;
    }

    if (ey > h) {
        // reset target
        ey = 0;
        ex = rand() % (w - 2) + 1;
    }

    static int speed = 0;
    if (speed < 10) speed++;
    else {
        ey++;
        speed = 0;
    }
}

void updateWithInput() {
    char input;
    if (_kbhit()) {
        input = _getch();
        if (input == 'a' && px > 0) px--;
        if (input == 'd' && px < w) px++;
        if (input == 'w' && py > 5) py--;
        if (input == 's' && py < h) py++;
        if (input == ' ' && bx == -1 && by == -1) {
            bx = px;
            by = py - 1;
        }
    }
}

void HideCursor() {
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

int main() {
    HideCursor();
    startup();
    while (1) {
        show();
        updateWithoutInput();
        updateWithInput();

        if (score == 10) {
            break;
        }
    }

    printf("\n\n\n\n\nDONE TARGET PRACTISE\n\n\n\n\n");
    return 0;
}