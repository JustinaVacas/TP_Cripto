IDIR =./include
CC=gcc
CFLAGS=-g --std=c11 -pedantic -Wall -Wextra -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200809L -fsanitize=address -I$(IDIR)

ODIR=./build
LIBS=-lcrypto -lssl

TARGET:=stegobmp

SRC = $(wildcard *.c)
DEPS = $(patsubst %,$(IDIR)/%,$(wildcard *.h))
OBJ = $(patsubst %,$(ODIR)/%,$(SRC:.c=.o))

all: compile link

compile: $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

stegobmp: %.o
	$(CC) $(CFLAGS) $< -o $@

link: $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $(TARGET)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~