#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  bool eFlag;
  bool errors_on;
  bool h_flag;
  bool o_flag;
  bool f_flag;
  bool i_flag;
  int v_flag;
  bool l_flag;
  bool c_flag;
  bool n_flag;
} Flags;

void Grep(Flags flags, char *argv[], char *pattern[], int *count_pattern,
          char *files[], int *count_files, FILE *fp);

int CustomError(bool s_flag) {
  if (!s_flag) {
    perror("Error message:");
  }
  return 0;
}

Flags ReadFLags(int argc, char *argv[], char *pattern[], int *count_pattern,
                char *files[], int *count_files) {
  struct option longOptions[] = {{0, 0, 0, 0}

  };
  // opterr = 0;  // turn off getopt errors
  int null_flag = 0;
  int currentFlag =
      getopt_long(argc, argv, "e:shof:vilcn", longOptions, &null_flag);
  Flags flags = {false, false, false, false, false,
                 false, 0,     false, false, false};
  for (; currentFlag != -1;
       currentFlag =
           getopt_long(argc, argv, "e:shof:vilcn", longOptions, &null_flag)) {
    switch (currentFlag) {
      case 'o':
        flags.o_flag = true;
        break;
      case 'e':
        flags.eFlag = true;
        pattern[*count_pattern] = optarg;
        (*count_pattern)++;
        break;
      case 'h':
        flags.h_flag = true;
        break;
      case 'f':
        flags.f_flag = true;
        files[*count_files] = optarg;
        (*count_files)++;
        break;
      case 'i':
        flags.i_flag = true;
        break;
      case 's':
        flags.errors_on = true;
        break;
      case 'v':
        flags.v_flag = REG_NOMATCH;
        break;
      case 'l':
        flags.l_flag = true;
        break;
      case 'c':
        flags.c_flag = true;
        break;
      case 'n':
        flags.n_flag = true;
        break;
      case '?':
        exit(0);
        break;
    }
  }
  if (!flags.eFlag && !flags.f_flag) {
    pattern[*count_pattern] = argv[optind];
    (*count_pattern)++;
    optind++;
  }
  if (optind <= argc - 1 && flags.h_flag) {
    flags.h_flag = false;
  } else if (optind < argc - 1) {
    flags.h_flag = true;
  }

  return flags;
}

void GrepFile(Flags flags, int argc, char *argv[], char *pattern[],
              int *count_pattern, char *files[], int *count_files) {
  FILE *fp;
  for (; optind < argc; optind++) {
    if ((fp = fopen(argv[optind], "rb")) == NULL) {
      CustomError(flags.errors_on);
      continue;
    } else {
      Grep(flags, argv, pattern, count_pattern, files, count_files, fp);
      fclose(fp);
    }
  }
}

void Grep(Flags flags, char *argv[], char *pattern[], int *count_pattern,
          char *files[], int *count_files, FILE *fp) {
  regex_t preg;
  char *line = 0;
  char res_pattern[512];
  size_t len_line = 0;
  regmatch_t pmatch[1];
  regoff_t len;
  int count_line = 0, count = 0;
  if ((flags.v_flag && flags.o_flag) && (!flags.l_flag && !flags.c_flag)) {
    return;
  }
  if (*count_pattern != 0) strcpy(res_pattern, pattern[0]);
  for (int i = 1; i < *count_pattern; i++) {
    strcat(res_pattern, "|");
    strcat(res_pattern, pattern[i]);
  }
  if (flags.f_flag) {
    FILE *file;
    for (int i = 0; i < *count_files; i++) {
      if ((file = fopen(files[i], "rb")) == NULL) {
        CustomError(true);
        fclose(fp);
        exit(0);
      } else {
        while (getline(&line, &len_line, file) > 0) {
          if (line == NULL) {
            continue;
          }
          if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
          }
          strcat(res_pattern, "|");
          strcat(res_pattern, line);
        }
        fclose(file);
      }
    }
  }

  if (flags.i_flag) {
    if (regcomp(&preg, res_pattern, REG_EXTENDED | REG_NEWLINE | REG_ICASE) !=
        0) {
      CustomError(true);
    }
  } else {
    if (regcomp(&preg, res_pattern, REG_EXTENDED | REG_NEWLINE) != 0) {
      CustomError(true);
    }
  }
  while (getline(&line, &len_line, fp) > 0) {
    count++;
    if (!flags.o_flag) {
      if (regexec(&preg, line, 1, pmatch, 0) == flags.v_flag) {
        if (flags.l_flag) {
          printf("%s\n", argv[optind]);
          break;
        }
        if (flags.c_flag) {
          count_line++;
          continue;
        }
        if (flags.h_flag) {
          printf("%s:", argv[optind]);
        }
        if (flags.n_flag) {
          printf("%d:", count);
        }
        printf("%s", line);
        if (line[strlen(line) - 1] != '\n') {
          printf("\n");
        }
      }
    } else {
      if (flags.l_flag) {
        if (regexec(&preg, line, 1, pmatch, 0) == flags.v_flag) {
          printf("%s\n", argv[optind]);
          break;
        } else {
          continue;
        }
      }
      if (flags.c_flag) {
        if (regexec(&preg, line, 1, pmatch, 0) == flags.v_flag) {
          count_line++;
          continue;
        } else {
          continue;
        }
      }
      char *s = line;
      for (int i = 0;; i++) {
        if (regexec(&preg, s, 1, pmatch, 0)) break;
        // off = pmatch[0].rm_so + (s - line);
        len = pmatch[0].rm_eo - pmatch[0].rm_so;
        if (flags.h_flag) {
          printf("%s:", argv[optind]);
        }
        if (flags.n_flag) {
          printf("%d:", count);
        }
        char tmp[strlen(line)];
        memcpy(tmp, s + pmatch[0].rm_so, len);
        tmp[len] = 0;
        // int len = pmatch[0].rm_eo - pmatch[0].rm_so;
        printf("%s\n", tmp);

        s += pmatch[0].rm_eo;
      }
    }
  }
  if (flags.l_flag) {
  } else if (flags.c_flag) {
    if (flags.h_flag) {
      printf("%s:", argv[optind]);
    }
    printf("%d\n", count_line);
  }
  free(line);
  regfree(&preg);
}

int main(int argc, char *argv[]) {
  char *pattern[argc], *files[argc];
  int count_pattern = 0, count_file = 0;
  Flags flags =
      ReadFLags(argc, argv, pattern, &count_pattern, files, &count_file);
  GrepFile(flags, argc, argv, pattern, &count_pattern, files, &count_file);
  return 0;
}