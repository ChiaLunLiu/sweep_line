CFLAGS = -Wall -std=c++11 -O2 -g -DDEBUG
all:
	g++ ${CFLAGS} segments_intersection.cpp main.cpp line_intersect.cpp rbtree.c
clean:
	rm -f a.out *.o
