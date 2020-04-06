#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

int px, py;
int h, w;

void startup() {
	h = 20;
	w = 30;
	px = 15;
	py = 19;
}

void show() {
	system("cls");
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			if (i == px && j == py) {
				printf("*");
			}
			else {
				printf(" ");
			}
			printf("\n");
		}
	}
}