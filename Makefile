# define flags
CC = gcc
CFLAGS = -Wall -g
TARGET = allocate
OBJS = allocate.o data.o schedule.o memory.o

# default rule
all: $(TARGET)

# link obj files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# compile rule
allocate.o: allocate.c data.h schedule.h memory.h
	$(CC) $(CFLAGS) -c allocate.c

data.o: data.c data.h
	$(CC) $(CFLAGS) -c data.c

schedule.o: schedule.c schedule.h
	$(CC) $(CFLAGS) -c schedule.c

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c

# clean rule
clean:
	rm -f $(OBJS) $(TARGET)