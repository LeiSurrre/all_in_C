#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

int main() {
    int step = 30;
    initgraph(600, 600);
    setbkcolor(YELLOW); // background color
    cleardevice(); // clear window with background color

    setlinestyle(PS_SOLID, 2); // 2 px solid line
    setcolor(BLACK);
    for (int i = 1; i < 20; i++) {
        line(i * step, step, i * step, 19 * step);
        line(step, i * step, 19 * step, i * step);
    }

    setcolor(BLACK);
    setfillcolor(BLACK);
    fillcircle(4 * step, 4 * step, 3);
    fillcircle(16 * step, 4 * step, 3);
    fillcircle(4 * step, 16 * step, 3);
    fillcircle(16 * step, 16 * step, 3);
    fillcircle(10 * step, 10 * step, 3);
    fillcircle(4 * step, 10 * step, 3);
    fillcircle(10 * step, 4 * step, 3);
    fillcircle(16 * step, 10 * step, 3);
    fillcircle(10 * step, 16 * step, 3);

    _getch();
    closegraph();
    return 0;
}