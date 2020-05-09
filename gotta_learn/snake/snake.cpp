#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define H 25
#define W 25
#define INIT_LEN 5

int direction; // 0 up, 1 right, 2 down, 3 left
int foodx, foody;
int headx, heady;
int tailx, taily;
int tail;
int canvas[H][W] = {};
// 0 space, -1 boundary, -2 food, 1 head, [2,inf) body

void startup() {
    int i, j, k;

    // boundary
    for (i = 0; i < W; i++) {
        canvas[0][i] = -1;
        canvas[H - 1][i] = -1;
    }
    for (j = 0; j < H; j++) {
        canvas[j][0] = -1;
        canvas[j][W - 1] = -1;
    }

    // snake
    headx = W / 2;
    heady = H / 2;
    canvas[heady][headx] = 1;
    for (k = 1; k < INIT_LEN; k++) {
        canvas[heady][headx - k] = k + 1;
    }
    tailx = headx - INIT_LEN + 1;
    taily = heady;
    tail = INIT_LEN;

    // direction
    direction = 1;

    //food
    srand(time(NULL));
    foodx = rand() % (W - 5) + 2;
    foody = rand() % (H - 5) + 2;
    canvas[foody][foodx] = -2;
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
            } else if (canvas[j][i] == -1) {
                printf("#");
            } else if (canvas[j][i] == 1) {
                printf("@");
            } else if (canvas[j][i] > 1) {
                printf("*");
            } else if (canvas[j][i] == -2) {
                printf("o");
            }
        }
        printf("\n");
    }
    Sleep(100);
}

void move() {
    int i, j;
    int newTailx, newTaily;
    for (j = 1; j < H - 1; j++) {
        for (i = 0; i < W; i++) {
            if (canvas[j][i] > 0) {
                if (canvas[j][i] == tail - 1) {
                    newTailx = i;
                    newTaily = j;
                }
                canvas[j][i]++;
            }
        }
    }

    if (direction == 0) { // up
        heady--;
    } else if (direction == 1) { // right
        headx++;
    } else if (direction == 2) { // down
        heady++;
    } else if (direction == 3) { // left
        headx--;
    }

    // eat food
    if (headx == foodx && heady == foody) {
        canvas[foody][foodx] = 0;
        foodx = rand() % (W - 5) + 2;
        foody = rand() % (H - 5) + 2;
        canvas[foody][foodx] = -2;
        tail++;
    } else {
        canvas[taily][tailx] = 0;
        tailx = newTailx;
        taily = newTaily;
    }

    // collision
    if (canvas[heady][headx] > 0 || canvas[heady][headx] == -1) {
        printf("GAME OVER!!!\n");
        Sleep(2000);
        system("pause");
        exit(0);
    } else {
        canvas[heady][headx] = 1;
    }
}

void updateWithoutInput() {
    move();
}

void updateWithInput() {
    char input;
    if (_kbhit()) {
        input = _getch();
        if (input == 'w') {
            if (direction != 2) {
                direction = 0;
                move();
            }
        } else if (input == 'd') {
            if (direction != 3) {
                direction = 1;
                move();
            }
        } else if (input == 's') {
            if (direction != 0) {
                direction = 2;
                move();
            }
        } else if (input == 'a') {
            if (direction != 1) {
                direction = 3;
                move();
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