# DEBUG can be left 0, or set to 1 for some very limited fault output
DEBUG = 0

# CONFIG default is "config.yaml", but can be overwritten using 
# `make generate CONFIG=something_else.yml`
CONFIG ?= config.yaml 

PYTHON = /usr/bin/env python3 # TODO: is this portable?


.DEFAULT_GOAL = generate
generate : pi-protocol.h.j2 pi-messages.h.j2 generate_headers.py
	$(PYTHON) generate_headers.py $(CONFIG)

# for test-cases
CC = gcc

C_FLAGS = -Wall -Wpedantic -Werror
ifneq ($(DEBUG),0)
C_FLAGS += -g -O0
else
C_FLAGS += -O3
endif

tester: generate tester.c pi-protocol.h pi-messages.h
	$(CC) -g tester.c -o tester

clean : 
	$(RM) tester
	$(RM) pi-protocol.h
	$(RM) pi-messages.h
