#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

int main() {
    initgraph(640, 480);
    setcolor(YELLOW);
    setfillcolor(GREEN);
    fillcircle(100, 100, 20);
    _getch();
    closegraph();
    return 0;
}