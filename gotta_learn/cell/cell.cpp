#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define H 25
#define W 50

int cells[H][W];

void gotoxy(int x, int y) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(handle, pos);
}

void startup() {
	int i, j;
	srand(time(NULL));
	for (i = 0; i < W; i++) {
		for (j = 0; j < H; j++) {
			cells[i][j] = rand() % 2;
		}
	}
}

void show() {
	gotoxy(0, 0);
	int i, j;
	for (j = 0; j < H; j++) {
		for (i = 0; i < W; i++) {
			if (cells[i][j] == 0) {
				printf(" ");
			}
			else {
				printf("*");
			}
		}
		printf("\n");
	}
	Sleep(100);
}

int main() {
	startup();
	while (1) {
		show();
	}

	return 0;
}