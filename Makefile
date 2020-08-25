CC=gcc
CFLAGS= -g -Wall
TARGET = sched.out
OBJS = gen_task.o main.o test_ttc.o utils.o bin_packing.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

gen_task.o : gen_task.c
	$(CC) -c -o gen_task.o gen_task.c

test_partftl.o : test_partftl.c
	$(CC) -c -o test_partftl.o test_partftl.c

test_ttc.o : test_ttc.c
	$(CC) -c -o test_ttc.o test_ttc.c

utils.o : utils.c
	$(CC) -c -o utils.o utils.c

bin_packing.o : bin_packing.c
	$(CC) -c -o bin_packing.o bin_packing.c

main.o : main.c
	$(CC) -c -o main.o main.c

clean:
	rm -f *.o
	rm $(OBJECT) $(TARGET)

