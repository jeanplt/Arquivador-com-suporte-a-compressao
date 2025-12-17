# Makefile

CFLAGS = -Wall -Wextra -std=c11 -O2 # flags de compilação

CC = gcc

# arquivos-objeto
objects = options.o lz.o vina.o

# regra padrão
all: vinac

options.o: options.c options.h

lz.o: lz.c lz.h

vina.o: vina.c options.h lz.h

# executável
vinac: $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)

# limpeza
clean:
	rm -f *.o vinac

.PHONY: all clean