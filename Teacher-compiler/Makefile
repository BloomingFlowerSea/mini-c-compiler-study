# Makefile for Simple C Compiler

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
TARGET = scc
SRC = simple_c_compiler.c

# 默认目标
all: $(TARGET)

# 编译编译器
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# 测试编译器
test: $(TARGET)
	./$(TARGET) test_program.c --ast --tokens

# 生成汇编代码
asm: $(TARGET)
	./$(TARGET) test_program.c -o test_output.asm

# 清理
clean:
	rm -f $(TARGET) *.asm

# 帮助
help:
	@echo "Simple C Compiler Makefile"
	@echo "Targets:"
	@echo "  all   - Build the compiler (default)"
	@echo "  test  - Run compiler with test program"
	@echo "  asm   - Generate assembly output"
	@echo "  clean - Remove built files"
	@echo "  help  - Show this help message"

.PHONY: all test asm clean help
