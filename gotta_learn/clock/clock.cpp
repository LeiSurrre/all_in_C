#include <graphics.h> // EasyX library https://docs.easyx.cn/en-us/intro
#include <math.h>
#include <conio.h>

#define HIGH 640
#define WIDTH 640
#define PI 3.1415926

int main() {
    initgraph(WIDTH, HIGH);
    int centerx = WIDTH / 2;
    int centery = HIGH / 2;
    int secLength = WIDTH / 5;
    int minLength = WIDTH / 6;
    int hourLength = WIDTH / 7;
    
    int secEndx, secEndy, minEndx, minEndy, hourEndx, hourEndy;
    float secAngle, minAngle, hourAngle;

    SYSTEMTIME ti; // local time

    BeginBatchDraw(); //flush the animation
    while (1) {
        // clock boundary
        setlinestyle(PS_SOLID, 1);
        setcolor(WHITE);
        circle(centerx, centery, WIDTH / 4);

        // readings
        int x, y, i;
        for (i = 0; i < 60; i++) {
            x = centerx + int(WIDTH / 4.3 * sin(PI * 2 * i / 60));
            y = centery + int(WIDTH / 4.3 * cos(PI * 2 * i / 60));

            if (i % 15 == 0) {
                bar(x - 5, y - 5, x + 5, y + 5);
            }
            else if (i % 5 == 0) {
                circle(x, y, 3);
            }
            else {
                putpixel(x, y, WHITE);
            }
        }

        outtextxy(centerx - 25, centery + 100, L"CLOCK");

        GetLocalTime(&ti);
        secAngle = ti.wSecond * 2 * PI / 60;
        minAngle = ti.wMinute * 2 * PI / 60 + secAngle / 60;
        hourAngle = ti.wHour * 2 * PI / 12 + minAngle / 12;

        secEndx = centerx + secLength * sin(secAngle);
        secEndy = centery - secLength * cos(secAngle);
        minEndx = centerx + minLength * sin(minAngle);
        minEndy = centery - minLength * cos(minAngle);
        hourEndx = centerx + hourLength * sin(hourAngle);
        hourEndy = centery - hourLength * cos(hourAngle);

        setlinestyle(PS_SOLID, 2);
        setcolor(YELLOW);
        line(centerx, centery, secEndx, secEndy);

        setlinestyle(PS_SOLID, 4);
        setcolor(BLUE);
        line(centerx, centery, minEndx, minEndy);

        setlinestyle(PS_SOLID, 6);
        setcolor(RED);
        line(centerx, centery, hourEndx, hourEndy);

        FlushBatchDraw(); //flush the animation
        Sleep(10);

        setcolor(BLACK);
        setlinestyle(PS_SOLID, 2);
        line(centerx, centery, secEndx, secEndy);
        setlinestyle(PS_SOLID, 4);
        line(centerx, centery, minEndx, minEndy);
        setlinestyle(PS_SOLID, 6);
        line(centerx, centery, hourEndx, hourEndy);
    }

    EndBatchDraw(); //flush the animation
    _getch();
    closegraph();
    return 0;
}