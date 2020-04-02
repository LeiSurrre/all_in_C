#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono> 

int main() {
  int x = 0;
  int y = 0;
  int ex = rand() % 29 + 1;
  int ey = rand() % 19 + 1;

  int vx = 1;
  int vy = 1;
  int left = 0;
  int right = 30;
  int top = 0;
  int bottom = 20;

  int i, j;
  while(1) {
  	x = x + vx;
  	y = y + vy;

  	// system("cls");
  	for(i=0;i<x;i++) {
  	  printf("\n");
  	}
  	for(j=0;j<y;j++) {
  	  printf(" ");
  	}
  	printf("o");
  	printf("\n");
  	std::this_thread::sleep_for (std::chrono::seconds(1));
  	//sleep(50);

  	if ((x == left) || (x == right)) {
  	  vx = -vx;
  	}
  	if ((y == top) || y == bottom) {
  	  vy = -vy;
  	}
  }

  return 0;
}