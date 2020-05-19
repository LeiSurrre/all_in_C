#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <time.h>
#include <math.h>
#include <conio.h>

#define HIGH 480
#define WIDTH 640
#define BALLN 10

int main() {
    int balls[BALLN][2];
    int ballv[BALLN][2];
    int radius = 20;
    int i, j;

    srand(time(NULL));
    for (i = 0; i < BALLN; i++) {
        balls[i][0] = rand() % int(WIDTH - 4 * radius) + 2 * radius;
        balls[i][1] = rand() % int(HIGH - 4 * radius) + 2 * radius;
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

            // fix ball out of boundary
            if (balls[i][0] < radius) balls[i][0] = radius;
            if (balls[i][1] < radius) balls[i][1] = radius;
            if (balls[i][0] > WIDTH - radius) balls[i][0] = WIDTH - radius;
            if (balls[i][1] > HIGH - radius) balls[i][1] = HIGH - radius;
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

        float minDistanceSquare[BALLN][2];
        for (i = 0; i < BALLN; i++) {
            minDistanceSquare[i][0] = 99999999; // distance square
            minDistanceSquare[i][1] = -1; // index
        }

        // calculate squared distance
        for (i = 0; i < BALLN; i++) {
            for (j = 0; j < BALLN; j++) {
                if (i != j) {
                    float distSquare = (balls[i][0] - balls[j][0]) * (balls[i][0] - balls[j][0])
                        + (balls[i][1] - balls[j][1]) * (balls[i][1] - balls[j][1]);
                    if (distSquare < minDistanceSquare[i][0]) {
                        minDistanceSquare[i][0] = distSquare;
                        minDistanceSquare[i][1] = j;
                    }
                }
            }
        }

        //collision check
        for (i = 0; i < BALLN; i++) {
            if (minDistanceSquare[i][0] <= 4 * radius * radius) { // (2*radius)^2
                j = minDistanceSquare[i][1];
                int temp;
                temp = ballv[i][0];
                ballv[i][0] = ballv[j][0];
                ballv[j][0] = temp;
                temp = ballv[i][1];
                ballv[i][1] = ballv[j][1];
                ballv[j][1] = temp;

                // avoid duplicated check
                minDistanceSquare[j][0] = 99999999;
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