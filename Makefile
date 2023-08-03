# CONFIG default is "config.yaml", but can be overwritten using 
# `make generate CONFIG=something_else.yml`
CONFIG ?= config.yaml 

PYTHON = /usr/bin/env python3 # TODO: is this portable?


.DEFAULT_GOAL = generate
generate : pi-protocol.h pi-messages.h

pi-protocol.h : templates/pi-protocol.h.j2 python/generate_headers.py config.yaml msgs/*.yaml
	$(PYTHON) python/generate_headers.py $(CONFIG) --protocol-only

pi-messages.h : templates/pi-messages.h.j2 python/generate_headers.py config.yaml msgs/*.yaml
	$(PYTHON) python/generate_headers.py $(CONFIG) --messages-only

# for test-cases
CC = gcc

C_FLAGS = -Wall -Wpedantic -Werror -g -O0

DEFINES = -DPI_USE_PRINT_MSG -DPI_STATS -DPI_DEBUG

tester: tests/tester.c pi-protocol.h pi-messages.h
	$(CC) $(C_FLAGS) $(DEFINES) tests/tester.c -o tester

clean : 
	$(RM) tester
	$(RM) pi-protocol.h
	$(RM) pi-messages.h
