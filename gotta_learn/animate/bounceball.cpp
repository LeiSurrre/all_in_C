#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

#define HIGH 480
#define WIDTH 640

int main() {
    float ballx, bally;
    float ballvx, ballvy;
    float radius;

    initgraph(WIDTH, HIGH);
    ballx = WIDTH / 2;
    bally = HIGH / 2;
    ballvx = 1;
    ballvy = 1;
    radius = 20;
    
    BeginBatchDraw(); //flush the animation
    while (1) {
        setcolor(BLACK);
        setfillcolor(BLACK);
        fillcircle(ballx, bally, radius);

        ballx = ballx + ballvx;
        bally = bally + ballvy;

        if (ballx <= radius || ballx >= WIDTH - radius) {
            ballvx = -ballvx;
        }
        if (bally <= radius || bally >= HIGH - radius) {
            ballvy = -ballvy;
        }

        setcolor(YELLOW);
        setfillcolor(GREEN);
        fillcircle(ballx, bally, radius);

        FlushBatchDraw(); //flush the animation
        Sleep(5);
    }

    EndBatchDraw(); //flush the animation
    closegraph();
    return 0;
}