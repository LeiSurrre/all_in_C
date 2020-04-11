#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

int h, w;
int px, py;
int bx, btop, bbot;
int score;

void gotoxy(int x, int y) {
    HANDLE handle = GetStdHnadle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle, pos);
}

void startup() {

}

void show() {
    gotoxy(0, 0);
}

void updateWithoutInput() {

}

void updateWithInput() {

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
