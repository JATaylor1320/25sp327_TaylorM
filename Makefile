CC = gcc 
LINKER = gcc
CFLAGS = -Wall -Wextra -O0 -g 
LDFLAGS =-g
LIBS=-lpthread
OBJS=worker_thread.o thread_pool.o main.o queue.o task.o
TARGETS=request_server test_queue

all: $(TARGETS)

request_server: $(OBJS)
	$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o request_server

queue.o: queue.c queue_internal.h queue.h common.h
	$(CC) $(CFLAGS) -c queue.c -o queue.o 

thread_pool.o: thread_pool.c thread_pool.h common.h queue.h worker_thread.h
	$(CC) $(CFLAGS) -c thread_pool.c -o thread_pool.o 

main.o: main.c common.h queue.h task.h thread_pool.h 
	$(CC) $(CFLAGS) -c main.c -o main.o 

task.o: task.c task.h
	$(CC) $(CFLAGS) -c task.c -o task.o

worker_thread.o: worker_thread.c worker_thread.h common.h queue.h task.h
	$(CC) $(CFLAGS) -c worker_thread.c -o worker_thread.o 

test_queue.o: test_queue.c queue_internal.h queue.h
	$(CC) $(CFLAGS) -c queue.c -o queue.o 

test_queue: test_queue.o queue.o 
	$(LINKER) $(LDFLAGS) test_queue.o queue.o -o test_queue -lcmocka 

clean: 
	rm -fr *.o $(OBJS) $(TARGETS)

