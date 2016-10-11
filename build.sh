#!/bin/sh
gcc -D_XOPEN_SOURCE=500 -lpthread -std=c99 -g -Wall -Wextra -Werror -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -o main main.c manager.c tools.c car.c
exit $?
