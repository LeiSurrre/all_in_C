#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <time.h>
#include <conio.h>

#define HIGH 480
#define WIDTH 640
#define BALLN 5

int main() {
    int balls[BALLN][2];
    int ballv[BALLN][2];
    int radius = 20;
    int i;

    srand(time(NULL));
    for (i = 0; i < BALLN; i++) {
        balls[i][0] = (i + 2) * radius * 3;
        balls[i][1] = HIGH / 2;
        ballv[i][0] = (rand() % 2) * 2 - 1;
        ballv[i][1] = (rand() % 2) * 2 - 1;
    }


    initgraph(WIDTH, HIGH);
    BeginBatchDraw(); //flush the animation

    while (1) {
        setcolor(BLACK);
        setfillcolor(BLACK);
        // reset previous color
        for (i = 0; i < BALLN; i++) {
            fillcircle(balls[i][0], balls[i][1], radius);
        }

        // update ball position
        for (i = 0; i < BALLN; i++) {
            balls[i][0] += ballv[i][0];
            balls[i][1] += ballv[i][1];
        }

        // hit wall
        for (i = 0; i < BALLN; i++) {
            if (balls[i][0] <= radius || balls[i][0] >= WIDTH - radius) {
                ballv[i][0] = -ballv[i][0];
            }
            if (balls[i][1] <= radius || balls[i][1] >= HIGH - radius) {
                ballv[i][1] = -ballv[i][1];
            }
        }

        setcolor(YELLOW);
        setfillcolor(GREEN);
        // new circle
        for (i = 0; i < BALLN; i++) {
            fillcircle(balls[i][0], balls[i][1], radius);
        }

        FlushBatchDraw(); //flush the animation
        Sleep(5);
    }

    EndBatchDraw(); //flush the animation
    closegraph();
    return 0;
}