#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

int main() {
  int i,j;
  int x = 10;
  int y = 5;
  char input;
  int fire = 0;
  int score = 0;

  srand(time(NULL));
  int ex = rand() % 18 + 1;

  while(1) {
    system("cls");
    
    // target
    for (i = 0; i < ex; i++) {
    printf(" ");
    }
    printf("  x\n");

    // beam
    if (!fire) {
      for (j = 0; j < y; j++) {
        printf("\n");
      }
    } else {
      for (j = 0; j < y; j++) {
        for (i = 0; i < x; i++) {
          printf(" ");
        }
        printf("  |\n");
      }

      if (x == ex) {
          score++;
          ex = rand() % 18 + 1;
          fire = 0;
      }
    }

    // jet
    for (i = 0; i < x; i++) {
      printf(" ");
    }
    printf("  *\n");
    for (i = 0; i < x; i++) {
      printf(" ");
    }
    printf("*****\n");
    for (i = 0; i < x; i++) {
      printf(" ");
    }
    printf(" / \\ \n");

    // score
    if (score == 10) {
      break;
    }
    printf("\n SCORE: %d \n", score);
    
    // move
    if (_kbhit()) {
      input = _getch();
      if (input == 'a' && x > 0) x--;
      if (input == 'd' && x < 20) x++;
      if (input == 'w' && x > 5) y--;
      if (input == 's' && y < 10) y++;
      if (input == ' ') fire = 1 - fire;
    }
  }

  printf("DONE TARGET PRACTISE\n");
  return 0;
}