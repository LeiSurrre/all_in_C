#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

int h, w;
int bx, by;
int vx, vy;

void gotoxy(int x, int y) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle, pos);
}

void startup() {
    h = 15;
    w = 20;
    bx = w/2;
    by = 0;
    vx = 1;
    vy = 1;
}

void show() {
    gotoxy(0, 0);
}

void updateWithoutInput() {
}

void updateWithInput() {

}

int main() {
    startup();
    while (1) {
        show();
        updateWithoutInput();
        updateWithInput();
    }

    return 0;
}