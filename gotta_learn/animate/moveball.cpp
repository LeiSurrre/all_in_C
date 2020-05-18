#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

int main() {
    initgraph(640, 480);
    for (int i = 100; i < 540; i = i + 20) {
        // cover with black circle to clear graph
        setcolor(BLACK);
        setfillcolor(BLACK);
        fillcircle(i - 20, 100, 20);
        setcolor(YELLOW);
        setfillcolor(GREEN);
        fillcircle(i, 100, 20);
        Sleep(200);
    }
    closegraph();
    return 0;
}