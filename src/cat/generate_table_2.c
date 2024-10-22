#include <stdio.h>

int main() {
  for (int i = 0; i != 256; ++i) {
    if (i != 0 && (i % 10 == 0))
      printf("\n");
    else
      printf(" ");
    printf("\"%c\",", i);
  }
}