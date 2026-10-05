CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

CFLAGS += $(shell pkg-config --cflags raylib)
LDFLAGS += $(shell pkg-config --libs raylib)

TARGET = open
SRCS = src/open.c src/utils.c src/draw.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(OBJS): src/config.h src/utils.h src/draw.h

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS)

.PHONY: all clean
