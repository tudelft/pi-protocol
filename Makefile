# DEBUG can be left 0, or set to 1 for some very limited fault output
DEBUG = 0

# CONFIG default is "config.yaml", but can be overwritten using 
# `make generate CONFIG=something_else.yml`
CONFIG ?= config.yaml 

PYTHON = /usr/bin/env python3 # TODO: is this portable?


.DEFAULT_GOAL = generate
generate : pi-protocol.h pi-messages.h

pi-protocol.h : templates/pi-protocol.h.j2 python/generate_headers.py
	$(PYTHON) python/generate_headers.py $(CONFIG) --protocol-only

pi-messages.h : templates/pi-messages.h.j2 python/generate_headers.py
	$(PYTHON) python/generate_headers.py $(CONFIG) --messages-only

# for test-cases
CC = gcc

C_FLAGS = -Wall -Wpedantic -Werror
ifneq ($(DEBUG),0)
C_FLAGS += -g -O0
else
C_FLAGS += -O3
endif

tester: tests/tester.c pi-protocol.h pi-messages.h
	$(CC) -g tests/tester.c -o tester

clean : 
	$(RM) tester
	$(RM) pi-protocol.h
	$(RM) pi-messages.h
