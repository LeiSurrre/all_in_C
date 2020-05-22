#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <conio.h>

#define HIGH 480
#define WIDTH 640
#define COINN 5

int barx, bary;
int barh, barw;
int bar_left, bar_right, bar_top, bar_bot;

int score;

void startup() {
    barh = HIGH / 20;
    barw = WIDTH / 4;
    barx = WIDTH / 2;
    bary = HIGH - barh / 2;
    bar_left = barx - barw / 2;
    bar_right = bar_left + barw;
    bar_top = bary - barh / 2;
    bar_bot = bar_top + barh;
}

void clear() {
    setcolor(BLACK);
    setfillcolor(BLACK);

    fillrectangle(bar_left, bar_top, bar_right, bar_bot);
}

void show() {
    setcolor(YELLOW);
    setfillcolor(GREEN);

    fillrectangle(bar_left, bar_top, bar_right, bar_bot);
}

void updateWithoutInput() {

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
    return (score == 100);
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