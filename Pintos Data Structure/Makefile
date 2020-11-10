CC = gcc
TARGET = testlib
OBJECTS = debug.o hex_dump.o bitmap.o hash.o list.o main.o 

all = $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJECTS)
