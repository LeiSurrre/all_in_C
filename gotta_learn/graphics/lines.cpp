#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

int main() {
    initgraph(640, 480);
    for (int i = 0; i <= 480; i = i + 48) {
        line(0, i, 640, i);
    }
    _getch();
    closegraph();
    return 0;
}