# define flags
CC = gcc
CFLAGS = -Wall -g
TARGET = allocate
OBJS = allocate.o process.o schedule.o

# default rule
all: $(TARGET)

# link obj files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# compile rule
allocate.o: allocate.c process.h schedule.h
	$(CC) $(CFLAGS) -c allocate.c

process.o: process.c process.h
	$(CC) $(CFLAGS) -c process.c

schedule.o: schedule.c schedule.h
	$(CC) $(CFLAGS) -c schedule.c

# clean rule
clean:
	rm -f $(OBJS) $(TARGET)