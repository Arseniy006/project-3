CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -I.
LDFLAGS = -lbsd
SRC = main.c mongoose/mongoose.c input/input.c
OBJ = $(SRC:.c=.o)
EXEC = server

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
