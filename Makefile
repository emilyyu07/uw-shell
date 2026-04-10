CC = gcc
CFLAGS = -Wall -Wextra -g -I include
SRCS = src/main.c src/builtins.c src/executor.c src/parser.c src/repl.c src/utils.c
TARGET = uw-shell

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)