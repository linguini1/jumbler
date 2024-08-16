CC = gcc
OUT = jumbler

SRCDIR = $(abspath ./src)
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS = -Wall -Wextra

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJS)
	$(CC) $^ -o $(OUT)

clean:
	@rm $(OBJS)
	@rm $(OUT)
