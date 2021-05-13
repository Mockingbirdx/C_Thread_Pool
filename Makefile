CC=gcc
CFLAGS=-O2

test:main.o thread_pool.o
	gcc main.o thread_pool.o -o test -pthread

mian.o: main.c pthread.h
	gcc miac.c -o main.o

thread_pool: thread_pool.c thread_pool.h
	gcc thread_pool.c -o thread_pool.o 