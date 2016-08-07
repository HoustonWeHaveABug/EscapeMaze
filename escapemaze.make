ESCAPEMAZE_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

escapemaze.exe: escapemaze.o
	gcc -o escapemaze.exe escapemaze.o

escapemaze.o: escapemaze.c escapemaze.make
	gcc ${ESCAPEMAZE_C_FLAGS} -o escapemaze.o escapemaze.c
