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

test_partftl.o : test_partftl.c
	$(CC) -c -o test_partftl.o test_partftl.c

clean:
	rm -f *.o
	rm $(OBJECT) $(TARGET)

