CC = gcc
CFLAGS = -Wall -g -I../include
LDFLAGS = -L../lib -lrequest -pthread
TARGET = http_proxy  
OBJS = main.o http_proxy.o 

all: $(TARGET)

$(TARGET): $(OBJS) ../lib/librequest.a 
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: main.c http_proxy.h ../include/common.h ../include/thread_pool.h ../include/queue.h
	$(CC) $(CFLAGS) -c main.c 

http_proxy.o: http_proxy.c http_proxy.h ../include/common.h 
	$(CC) $(CFLAGS) -c http_proxy.c 

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean