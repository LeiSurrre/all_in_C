#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

int main() {
  int x = 0;
  int y = 0;
  srand(time(NULL));
  int ex = rand() % 29;
  int ey = rand() % 19;

  int vx = 1;
  int vy = 1;
  int left = 0;
  int right = 50;
  int top = 0;
  int bottom = 20;

  int i, j;
  while(1) {
    system("cls");

    // top boundary
    printf("+");
    for(i=left;i<right;i++) {
      printf("-");
    }
    printf("+\n");

    x = x + vx;
    y = y + vy;

    // everything inside
    for (j=top;j<bottom;j++) {
      // left boundary
      printf("|");
      for (i=left;i<right;i++) {
        if (i == x && j==y) {
          printf("o");
        } else if (i == ex && j == ey) {
          printf("x");
        } else {
          printf(" ");
        }
      }
      // right boundary
      printf("|\n");
    }

    // bottom boundary
    printf("+");
    for(i=left;i<right;i++) {
      printf("-");
    }
    printf("+\n");

    //std::this_thread::sleep_for (std::chrono::seconds(1));
    Sleep(50);
    if (x == ex && y == ey) {
      printf("The target is reached.");
      break;
    }

  	if ((x == left) || (x == right - 1)) {
  	  vx = -vx;
  	  printf("\a");
  	}
  	if ((y == top) || y == bottom - 1) {
      vy = -vy;
   	  printf("\a");
    }
  }

  return 0;
}