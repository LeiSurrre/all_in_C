#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

int h, w;
int px, py;
int b1x, b1top, b1bot;
int b2x, b2top, b2bot;
int b3x, b3top, b3bot;
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
    w = 50;
    px = 5;
    py = h / 2;
    // bars
    int gap;
    srand(time(NULL));
    b1x = w;
    b1top = (rand() % 10) + 10;
    gap = (rand() % 3) + 4;
    b1bot = b1top - gap;
    b2x = w + 20;
    b2top = (rand() % 10) + 10;
    gap = (rand() % 3) + 4;
    b2bot = b2top - gap;
    b3x = w + 40;
    b3top = (rand() % 10) + 10;
    gap = (rand() % 3) + 4;
    b3bot = b3top - gap;
    score = 0;
}

void show() {
    gotoxy(0, 0);
    int i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            // bird
            if (i == px && j == py) {
                printf("T");
            }
            // wall
            else if (i == b1x && (j > b1top || j < b1bot)) {
                printf("*");
            }
            else if (i == b2x && (j > b2top || j < b2bot)) {
                printf("*");
            }
            else if (i == b3x && (j > b3top || j < b3bot)) {
                printf("*");
            }
            else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("\nSCORE: %d\n", score);
}

void updateWithoutInput() {
    // bird fall
    py++;
    if (py == h) {
        printf("GAME OVER\nYOUR FINAL SCORE: %d\n", score);
        system("pause");
        exit(0);
    }
    // bar move
    b1x--;
    if (px == b1x) {
        if (py > b1bot && py < b1top) {
            score++;
        }
        else {
            printf("GAME OVER\nYOUR FINAL SCORE: %d\n", score);
            system("pause");
            exit(0);
        }
    }
    b2x--;
    if (px == b2x) {
        if (py > b2bot && py < b2top) {
            score++;
        }
        else {
            printf("GAME OVER\nYOUR FINAL SCORE: %d\n", score);
            system("pause");
            exit(0);
        }
    }
    b3x--;
    if (px == b3x) {
        if (py > b3bot && py < b3top) {
            score++;
        }
        else {
            printf("GAME OVER\nYOUR FINAL SCORE: %d\n", score);
            system("pause");
            exit(0);
        }
    }
    // bar reset
    int gap;
    if (b1x == 0) {
        b1x = w + 10;
        b1top = (rand() % 10) + 10;
        gap = (rand() % 3) + 4;
        b1bot = b1top - gap;
    }
    if (b2x == 0) {
        b2x = w + 10;
        b2top = (rand() % 10) + 10;
        gap = (rand() % 3) + 4;
        b2bot = b2top - gap;
    }
    if (b3x == 0) {
        b3x = w + 10;
        b3top = (rand() % 10) + 10;
        gap = (rand() % 3) + 4;
        b3bot = b3top - gap;
    }

    Sleep(150);
}

void updateWithInput() {
    char input;
    if (_kbhit()) {
        input = _getch();
        if (input == ' ') {
            py = py - 3;
            if (py < 0) py == 0;
        }
    }
}

void HideCursor() {
    CONSOLE_CURSOR_INFO cursor_info = { 1, 0 };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

int main() {
    HideCursor();
    startup();
    while (1) {
        show();
        updateWithoutInput();
        updateWithInput();
    }
    return 0;
}
