CC = gcc
CFLAGS = -Iinclude/libfst -Wall -O2
LDFLAGS = -lm -lz

FST_SRCS = include/libfst/fstapi.c include/libfst/lz4.c include/libfst/fastlz.c
FST_OBJS = $(FST_SRCS:%.c=build/%.o)

TARGET = build/fstdemo
SRCS = fstdemo.c
OBJS = $(SRCS:%.c=build/%.o)

all: build $(TARGET)

build:
	mkdir -p build/include/libfst

$(TARGET): $(OBJS) $(FST_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

run: build $(TARGET)
	./$(TARGET)

.PHONY: all clean run