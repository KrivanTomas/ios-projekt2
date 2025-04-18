CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic


SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

PROJ_NAME = proj2

.PHONY: all
all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

.PHONY: pack
pack:
	zip proj2.zip *.c *.h Makefile

.PHONY: clean
clean:
	rm -f *.o $(PROJ_NAME)
