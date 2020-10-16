#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define H 25
#define W 25

int canvas[H][W] = {0}; // 0 space, 1 man, 2 floor
int manx, many;
int floor[5];
int score;


void startup() {
    manx = 13;
    many = 9;
    canvas[many][manx] = 1;

    // initialize floors
    srand(time(NULL));
    int i, j;
    for (i = 0; i < 5; i++) {
        int curr = i * 5;
        floor[i] = curr;
        for (j = 0; j < W; j++) {
            canvas[curr][j] = 2;
        }
        int spaceNum = rand() % 3 + 1;
        int space;
        while (spaceNum > 0) {
            space = rand() % 5;
            for (j = 0; j < 5; j++) {
                canvas[curr][space * 5 + j] = 0;
            }
            spaceNum--;
        }
    }
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
    int i, j;
    for (j = 0; j < H; j++) {
        for (i = 0; i < W; i++) {
            if (canvas[j][i] == 0) {
                printf(" ");
            }
            else if (canvas[j][i] == 1) {
                printf("I");
            }
            else if (canvas[j][i] == 2) {
                printf("=");
            }
        }
        printf("\n");
    }
    Sleep(100);
}

void updateWithoutInput() {

}

void updateWithInput() {
    char input;
    if (_kbhit()) {
        input = _getch();
        if (input == 'd') {
            if (manx < W - 1) {
                if (canvas[many][manx + 1] != 2) {
                    manx = manx + 1;
                }
            }
        } else if (input == 'a') {
            if (manx > 0) {
                if (canvas[many][manx - 1] != 2) {
                    manx = manx + 1;
                }
            }
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
    }

    return 0;
}