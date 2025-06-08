CC = gcc

TARGET = DesktopSnake.exe

SRCS = DesktopSnake.c

CFLAGS = -Wall -g -O2 -mwindows -municode -std=c99

LDFLAGS = -lcomctl32 -lgdi32 -lshell32

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "compiled"

run: $(TARGET)
	@echo "running"
	./$(TARGET)

clean:
	@echo "cleaning"
	rm -f $(TARGET)

.PHONY: all run clean
