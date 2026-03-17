CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
TARGET = mini-c.exe
SRC = src/main.c src/lexer.c src/parser.c src/interpreter.c src/utils.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) examples/test.txt

clean:
	rm -f $(TARGET)
