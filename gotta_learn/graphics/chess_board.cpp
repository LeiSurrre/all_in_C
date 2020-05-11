#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

int main() {
    int step = 50;
    initgraph(500, 500);
    setbkcolor(YELLOW); // background color
    cleardevice(); // clear window with background color

    int i, j;
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            if ((i + j) % 2 == 1) {
                setfillcolor(BLACK);
                solidrectangle(i * step, j * step, (i + 1) * step, (j + 1) * step);
            }
            else {
                setfillcolor(WHITE);
                solidrectangle(i * step, j * step, (i + 1) * step, (j + 1) * step);
            }
        }
    }

    _getch();
    closegraph();
    return 0;
}