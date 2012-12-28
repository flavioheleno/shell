CC = gcc
FLAGS = -Wall -O2 -pipe
APP = shell

all: command job main
	$(CC) $(FLAGS) -o $(APP) *.o

command:
	$(CC) $(FLAGS) -c $@.c

job:
	$(CC) $(FLAGS) -c $@.c

main:
	$(CC) $(FLAGS) -c $@.c

clean:
	rm -f *~ *.o $(APP)

run: all
	./$(APP)
