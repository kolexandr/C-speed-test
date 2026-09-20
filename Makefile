CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDLIBS = -lcurl

TARGET = speedtest

OBJS = build/main.o \
       build/download.o

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

$(OBJS): build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)
