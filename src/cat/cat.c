#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "catflags.h"

typedef struct {
  bool numberNonBlank;
  bool markEndl;
  bool numberAll;
  bool squeeze;
  bool tab;
  bool printNon;
} Flags;

Flags ReadFLags(int argc, char *argv[]) {
  struct option longOptions[] = {{"number-nonblank", 0, NULL, 'b'},
                                 {"number", 0, NULL, 'n'},
                                 {"squeeze-blank", 0, NULL, 's'},
                                 {0, 0, 0, 0}

  };
  // opterr = 0;  // turn off getopt errors
  int currentFlag = getopt_long(argc, argv, "bevEnstT", longOptions, NULL);
  Flags flags = {
      false, false, false, false, false, false,
  };
  for (; currentFlag != -1;
       currentFlag = getopt_long(argc, argv, "bevEnstT", longOptions, NULL)) {
    switch (currentFlag) {
      case 'b':
        flags.numberNonBlank = true;
        break;
      case 'e':
        flags.markEndl = true;
        flags.printNon = true;
        break;
      case 'v':
        flags.printNon = true;
        break;
      case 'E':
        flags.markEndl = true;
        break;
      case 'n':
        flags.numberAll = true;
        break;
      case 's':
        flags.squeeze = true;
        break;
      case 't':
        flags.printNon = true;
        flags.tab = true;
        break;
      case 'T':
        flags.tab = true;
        break;
      case '?':
        exit(0);
        break;
    }
  }
  return flags;
}

void CatFile(Flags flags, int argc, char *argv[], char *table[static 256]) {
  int c = 0;
  FILE *fp;
  int count = 1;
  while (optind != argc) {
    if ((fp = fopen(argv[optind], "rb")) == NULL) {
      printf("cannot open %s", argv[optind]);
      optind++;
      continue;
    }
    if (flags.numberNonBlank && !flags.squeeze) {
      // int count = 1;
      char first = '\0';
      while (fread(&c, 1, 1, fp)) {
        if (c == EOF) {
          break;
        }
        if ((first == '\n' && c != '\n') || (count == 1 && c != '\n')) {
          printf("%6d\t", count);
          count++;
        }
        first = c;
        printf("%s", table[c]);
      }
    } else if (flags.numberAll && !flags.squeeze) {
      // int count = 1;
      char first = '\0';
      while (fread(&c, 1, 1, fp)) {
        if (c == EOF) {
          break;
        }
        if ((first == '\n' && c != '\n') || (count == 1) ||
            (first == '\n' && c == '\n')) {
          printf("%6d\t", count);
          count++;
        }
        first = c;
        printf("%s", table[c]);
      }

    } else if (flags.squeeze && flags.numberNonBlank) {
      // int count = 1;
      char first = '\0';
      char last = '\0';
      while (fread(&c, 1, 1, fp)) {
        if (c == EOF) {
          break;
        }
        if (first == '\n' && last == '\n' && c == '\n') {
          continue;
        }
        if ((last == '\n' && c != '\n') || (count == 1 && c != '\n')) {
          printf("%6d\t", count);
          count++;
        }
        first = last;
        last = c;
        printf("%s", table[c]);
      }
    } else if (flags.squeeze && flags.numberAll) {
      // int count = 1;
      char first = '\0';
      char last = '\0';
      while (fread(&c, 1, 1, fp)) {
        if (c == EOF) {
          break;
        }
        if (first == '\n' && last == '\n' && c == '\n') {
          continue;
        }
        if ((last == '\n' && c != '\n') || (count == 1) ||
            (last == '\n' && c == '\n')) {
          printf("%6d\t", count);
          count++;
        }
        first = last;
        last = c;
        printf("%s", table[c]);
      }
    } else if (flags.squeeze) {
      char first = '\0';
      char last = '\0';
      while (fread(&c, 1, 1, fp)) {
        if (c == EOF) {
          break;
        }
        if (first == '\n' && last == '\n' && c == '\n') {
          continue;
        }
        first = last;
        last = c;
        printf("%s", table[c]);
      }

    } else {
      while (fread(&c, 1, 1, fp)) {
        if (c == EOF) {
          break;
        }
        printf("%s", table[c]);
      }
    }
    optind++;
    fclose(fp);
  }
}

int main(int argc, char *argv[]) {
  Flags flags = ReadFLags(argc, argv);
  char *table[256];
  CatSetTable(table);
  if (flags.markEndl) CatSetEnd(table);
  if (flags.tab) CatSetTab(table);
  if (flags.printNon) CatSetNonPrint(table);
  CatFile(flags, argc, argv, table);
  return 0;
}