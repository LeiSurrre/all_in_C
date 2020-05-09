#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define H 25
#define W 50
#define T 5

int px, py;
int bx, by;
int target[T][2];
int canvas[H][W] = {0};

int score;
int total;
int bulletNum;
int targetSpd;

void startup() {
    // 0 space, 1 jet, 2 bullet, 3 target

    // jet
    px = W/2;
    py = H-1;
    canvas[py][px] = 1;

    // target
    srand(time(NULL));
    int k;
    for (k = 0; k < T; k++) {
        target[k][0] = rand() % W;
        target[k][1] = rand() % 2;
        canvas[target[k][1]][target[k][0]] = 3;
        total++;
    }
    targetSpd = 20;

    // bullet
    bulletNum = 0;

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
    for (j = 0; j < H; j++) {
        for (i = 0; i < W; i++) {
            if (canvas[j][i] == 0) {
                printf(" ");
            } else if (canvas[j][i] == 1) {
                printf("*");
            } else if (canvas[j][i] == 2) {
                printf("|");
            } else if (canvas[j][i] == 3) {
                printf("x");
            }
        }
        printf("\n");
    }

    int rate = total == 5 ? 0 : 100 * score / (total - 5);
    printf("SCORE: %d\nTOTAL TARGET SPAWN: %d\nHIT RATE: %d%% \n", score, total, rate);
    Sleep(20);
}

void updateWithoutInput() {
    int i, j, k;
    for (j = 0; j < H; j++) {
        for (i = 0; i < W; i++) {
            // check bullet hit
            if (canvas[j][i] == 2) {
                for (k = 0; k < T; k++) {
                    if (i == target[k][0] && j == target[k][1]) {
                        score++;
                        if (score % 5 == 0 && targetSpd > 5) {
                            targetSpd--;
                        }
                        if (score % 5 == 0 && bulletNum < 5) {
                            bulletNum++;
                        }
                        canvas[j][i] = 0;
                        target[k][0] = rand() % W;
                        target[k][1] = rand() % 2;
                        canvas[target[k][1]][target[k][0]] = 3;
                        total++;
                    }
                }
            }

            // bullet move
            if (canvas[j][i] == 2) {
                if (j > 0) {
                    canvas[j - 1][i] = 2;
                }
                canvas[j][i] = 0;
            }
        }
    }

    // target speed control
    static int s = 0;
    if (s < targetSpd) s++;

    for (k = 0; k < T; k++) {
        if (px == target[k][0] && py == target[k][1]) {
            printf("!!!MISSION FAILED!!!\n");
            Sleep(3000);
            system("pause");
            exit(0);
        }

        if (target[k][1] == H) {
            target[k][0] = rand() % W;
            target[k][1] = rand() % 2;
            canvas[target[k][1]][target[k][0]] = 3;
            total++;
        }

        // target move
        if (s == targetSpd) {
            for (k = 0; k < T; k++) {
                canvas[target[k][1]][target[k][0]] = 0;
                target[k][1]++;
                canvas[target[k][1]][target[k][0]] = 3;
                s = 0;
            }
        }
    }

}

void updateWithInput() {
    char input;
    if (_kbhit()) {
        input = _getch();
        if (input == 'a' && px > 0) {
            canvas[py][px] = 0;
            if (px > 0) px--;
            canvas[py][px] = 1;
        }
        else if (input == 'd' && px < W - 1) {
            canvas[py][px] = 0;
            if (px < W - 1) px++;
            canvas[py][px] = 1;
        }
        else if (input == 'w' && py > 5) {
            canvas[py][px] = 0;
            if (py > 0) py--;
            canvas[py][px] = 1;
        }
        else if (input == 's' && py < H - 1) {
            canvas[py][px] = 0;
            if (py < H - 1) py++;
            canvas[py][px] = 1;
        }
        else if (input == ' ') {
            int left = px - bulletNum;
            int right = px + bulletNum;
            if (left < 0) left = 0;
            if (right > W - 1) right = W - 1;
            int k;
            for (k = left; k <= right; k++) {
                canvas[py - 1][k] = 2;
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

        if (score == 100) {
            break;
        }
    }

    printf("\n\n\n\n\nDONE TARGET PRACTISE\n\n\n\n\n");
    printf("SCORE: %d\nTOTAL TARGET SPAWN: %d\nHIT RATE: %d%%\n", score, total, (100 * score / (total - 5)));
    return 0;
}