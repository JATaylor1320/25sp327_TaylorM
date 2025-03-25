all: parent child

parent: parent.c 
	gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -o parent parent.c

child: child.c 
	gcc -Wall -Wextra  -std=c99 -D_POSIX_C_SOURCE=200809L -o child child.c 

run: parent 
	./parent 
	./send_signal.sh

clean: 
	rm -f parent child