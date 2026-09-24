CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDLIBS = -lcurl -lcjson

TARGET = speedtest

OBJS = build/download.o \
	   build/upload.o \
       build/utils.o \
	   build/location.o \
	   build/server.o \
	   build/main.o

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

$(OBJS): build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)
	rm -rf build 

.PHONY: clean