#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

#define HIGH 480
#define WIDTH 640
#define BRICKN 10

int ballx, bally;
int ballvx, ballvy;
int radius;
int barx, bary;
int barh, barw;
int bar_left, bar_right, bar_top, bar_bot;

int bricks[BRICKN] = {1};
int brickh, brickw;

void startup() {
    ballx = WIDTH / 2;
    bally = HIGH / 2;
    ballvx = 1;
    ballvy = 1;
    radius = 20;

    barh = HIGH / 20;
    barw = WIDTH / 4;
    barx = WIDTH / 2;
    bary = HIGH - barh / 2;
    bar_left = barx - barw / 2;
    bar_right = bar_left + barw;
    bar_top = bary - barh / 2;
    bar_bot = bar_top + barh;

    brickw = WIDTH / BRICKN;
    brickh = HIGH / BRICKN;
}

void clear() {
    setcolor(BLACK);
    setfillcolor(BLACK);
    fillcircle(ballx, bally, radius);
    fillrectangle(bar_left, bar_top, bar_right, bar_bot);

    int i, brick_left, brick_right, brick_top, brick_bot;
    for (i = 0; i < BRICKN; i++) {
        brick_left = i * brickw;
        brick_right = brick_left + brickw;
        brick_top = 0;
        brick_bot = brickh;
        if (!bricks[i]) {
            fillrectangle(brick_left, brick_top, brick_right, brick_bot);
        }
    }
}

void show() {
    setcolor(YELLOW);
    setfillcolor(GREEN);
    fillcircle(ballx, bally, radius);
    fillrectangle(bar_left, bar_top, bar_right, bar_bot);

    int i, brick_left, brick_right, brick_top, brick_bot;
    for (i = 0; i < BRICKN; i++) {
        brick_left = i * brickw;
        brick_right = brick_left + brickw;
        brick_top = 0;
        brick_bot = brickh;
        if (bricks[i]) {
            setcolor(WHITE);
            setfillcolor(RED);
            fillrectangle(brick_left, brick_top, brick_right, brick_bot);
        }
    }
}

void updateWithoutInput() {
    ballx = ballx + ballvx;
    bally = bally + ballvy;

    // bar and ball
    if (bally + radius >= bar_top && bally + radius < bar_top + barh / 3)
        || (bally - radius <= bar_bot && bally - radius > bar_bot - barh / 3) {
            if (ballx >= bar_left && ballx <= bar_left) {
                ballvy = - ballvy;
            }
    }

    // boundary and ball
    if (ballx <= radius || ballx >= WIDTH - radius) {
        ballvx = -ballvx;
    }
    if (bally <= radius || bally >= HIGH - radius) {
        ballvy = -ballvy;
    }

    // bricks and ball
    int i, brick_left, brick_right, brick_bot;
    for (i = 0; i < BRICKN; i++) {
        if (bricks[i]) {
            brick_left = i * brickw;
            brick_right = brick_left + brickw;
            brick_bot = brickh;

            if (bally == brick_bot + radius && ballx >= brick_left && ballx <= bally && ballvy == -1) {
                bricks[i] = 0;
                ballvy = -ballvy;
            }
        }
    }
}

void updateWithInput() {
    char input;
    if(_kbhit()) {
        input = _getch();
        if (input == 'a') {
            barx = barx - 15;;
            bar_left = barx - barw / 2;
            bar_right = bar_left + barw;
        }
        if (input == 'd') {
            barx = barx + 15;;
            bar_left = barx - barw / 2;
            bar_right = bar_left + barw;
        }
        if (input == 'w') {
            bary = bary - 10;
            bar_top = bary - barh / 2;
            bar_bot = bar_top + barh;
        }
        if (input == 's') {
            bary = bary + 10;
            bar_top = bary - barh / 2;
            bar_bot = bar_top + barh;
        }
    }
}

int checkEnding() {
    int i
    int end = 1;
    for (i = 0; i < BRICKN; i++) {
        if (bricks[i]) {
            end = 0;
            break;
        }

    }
    return end;
}

int main() {
    initgraph(WIDTH, HIGH);
    BeginBatchDraw(); //flush the animation
    startup();
    while (1) {
        clear();
        updateWithInput();
        updateWithoutInput();
        show();
        FlushBatchDraw(); //flush the animation
        Sleep(5);
        if (checkEnding()) {
            break;
        }
    }

    EndBatchDraw(); //flush the animation
    closegraph();
    return 0;
}