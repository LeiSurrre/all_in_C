#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

int main() {
    initgraph(256, 256);
    for (int i = 0; i < 256; i++) {
        setcolor(RGB(0, 0, i));
        line(0, i, 256, i);
    }
    _getch();
    closegraph();
    return 0;
}