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
	for (j = 0; j < H; j++) {
		for (i = 0; i < W; i++) {
			cells[j][i] = rand() % 2;
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
			} else {
				printf("*");
			}
		}
		printf("\n");
	}
	Sleep(100);
}

void updateWithoutInput() {
	int newCells[H][W];
	int n;
	int i, j;
	for (j = 1; j < H - 1; j++) {
		for (i = 1; i < W - 1; i++) {
			n = cells[j - 1][i - 1] + 
				cells[j - 1][i] +
				cells[j - 1][i + 1] +
				cells[j][i - 1] +
				cells[j][i + 1] +
				cells[j + 1][i - 1] +
				cells[j + 1][i] +
				cells[j + 1][i + 1];
			if (n == 3) {
				newCells[j][i] = 1;
			} else if (n == 2) {
				newCells[j][i] = cells[j][i];
			} else {
				newCells[j][i] = 0;
			}
		}
	}

	for (j = 1; j <= H - 1; j++) {
		for (i = 1; i <= W - 1; i++) {
			cells[j][i] = newCells[j][i];
		}
	}
}

int main() {
	startup();
	while (1) {
		show();
		updateWithoutInput();
	}

	return 0;
}