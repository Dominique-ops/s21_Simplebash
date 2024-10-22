all: s21_cat

s21_cat:catflags.o cat.o
	gcc catflags.o cat.o -o s21_cat

catflags.o:catflags.c
	gcc -Wall -Werror -Wextra -c  catflags.c

cat.o: cat.c
	gcc -Wall -Werror -Wextra -c  cat.c

clean:
	rm -rf *.o s21_cat