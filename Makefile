.RECIPEPREFIX = >
CC     = gcc
CFLAGS = -Wall -Wextra -Werror
SRC    = src/caixa_eletronico.c

all: caixa

caixa: $(SRC)
> $(CC) $(CFLAGS) -o caixa $(SRC)

run: caixa
> ./caixa

test:
> bash tests/run_tests.sh

clean:
> rm -f caixa contas.txt extrato.txt cedulas.txt

.PHONY: all run test clean
