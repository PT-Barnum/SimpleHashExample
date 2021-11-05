CFLAGS = -Wall -Wno-comment -Werror -g 
CC     = gcc $(CFLAGS)
SHELL  = /bin/bash
CWD    = $(shell pwd | sed 's/.*\///g')

all : \
	hashset_main \
# cleaning target to remove compiled program
clean :
	rm -f hashset_main *.o

help :
	@echo 'Typical usage is:'
	@echo '  > make                          # build all programs'
	@echo '  > make clean                    # remove all compiled items'

# hashset object & program files
hashset_main : hashset_main.o hashset_funcs.o
	$(CC) -o $@ $^

hashset_main.o : hashset_main.c hashset.h
	$(CC) -c $<

hashset_funcs.o : hashset_funcs.c hashset.h
	$(CC) -c $<
