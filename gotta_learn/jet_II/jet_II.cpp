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
    for (j = 0; j < h + 3; j++) {
        for (i = 0; i < w; i++) {
            if (i == px && j == py) {
                printf("*");
            } else if (i >= px - 2 && i <= px + 2 && j == py + 1) {
                printf("*");
            } else if (i == px - 1 && j == py + 2) {
                printf("/");
            } else if (i == px + 1 && j == py + 2) {
                printf("\\");
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

    // bullet hit target
    if (bx == ex && by == ey) {
        score++;

        // reset target
        ey = 0;
        ex = rand() % (w - 2) + 1;

        // reset bullet
        bx = -1;
        by = -1;
    }

    // target hit jet
    if ((ex == px && ey == py) ||
        (ex >= px - 2 && ex <= px + 2 && ey == py + 1) ||
        (ex == px - 1 && ey == py + 2) ||
        (ex == px + 1 && ey == py + 2)) {
        
        if (score > 0) score--;

        // reset target
        ey = 0;
        ex = rand() % (w - 2) + 1;
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
        if (input == 'd' && px < w - 1) px++;
        if (input == 'w' && py > 5) py--;
        if (input == 's' && py < h - 1) py++;
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