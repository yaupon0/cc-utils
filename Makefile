INC_DIR = include
CFLAGS = -Wall -Wextra -fno-strict-aliasing -I$(INC_DIR) -O3
CC = gcc
#CC = clang

owq_test: owq_test.c $(INC_DIR)/owq.h owq2.h
	$(CC) $(CFLAGS) owq_test.c -lpthread -o owq_test
owq2.h:	$(INC_DIR)/owq.h
	sed 's/owq_/owq2_/g' $(INC_DIR)/owq.h > owq2.h

markov:	markov.c $(INC_DIR)/dlinklist.h pair_ll.h follower_ll.h 
	$(CC) $(CFLAGS) markov.c -o markov

pair_ll.h:	$(INC_DIR)/dlinklist.h
	sed 's/dlist_/pair_/g' $(INC_DIR)/dlinklist.h > pair_ll.h
follower_ll.h:	$(INC_DIR)/dlinklist.h
	sed 's/dlist_/follower_/g' $(INC_DIR)/dlinklist.h > follower_ll.h

clean: 
	rm -f owq2.h owq_test
all:
