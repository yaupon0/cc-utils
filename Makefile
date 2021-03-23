INC_DIR = include
CFLAGS = -Wall -Wextra -fno-strict-aliasing -I$(INC_DIR) -O3
CC = gcc
#CC = clang

owq_test: owq_test.c $(INC_DIR)/owq.h owq2.h
	$(CC) $(CFLAGS) owq_test.c -lpthread -o owq_test
owq2.h:	$(INC_DIR)/owq.h
	sed 's/owq_/owq2_/g' $(INC_DIR)/owq.h > owq2.h

clean: 
	rm -f owq2.h owq_test
all:
