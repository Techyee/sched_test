CC=gcc
CFLAGS= -g -Wall
TARGET = sched.out
OBJS = gen_task.o main.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

gen_task.o : gen_task.c
	$(CC) -c -o gen_task.o gen_task.c

main.o : main.c
	$(CC) -c -o main.o main.c

clean:
	rm $(OBJECT) $(TARGET)

