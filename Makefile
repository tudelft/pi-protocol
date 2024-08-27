# Copyright 2024 Till Blaha (Delft University of Technology)
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along
# with this program.
#
# If not, see <https://www.gnu.org/licenses/>.

# CONFIG default is "config.yaml", but can be overwritten using 
# `make generate CONFIG=something_else.yml`
CONFIG ?= config.yaml 

PYTHON = /usr/bin/env python3 # TODO: is this portable?


.DEFAULT_GOAL = generate
generate : src/pi-protocol.h src/pi-messages.h src/pi-messages.c

src/pi-protocol.h : templates/pi-protocol.h.j2 python/generate.py config.yaml msgs/*.yaml
	$(PYTHON) python/generate.py $(CONFIG) --protocol-h-only --output-dir src/

src/pi-messages.h : templates/pi-messages.h.j2 python/generate.py config.yaml msgs/*.yaml
	$(PYTHON) python/generate.py $(CONFIG) --messages-h-only --output-dir src/

src/pi-messages.c : templates/pi-messages.c.j2 python/generate.py config.yaml msgs/*.yaml
	$(PYTHON) python/generate.py $(CONFIG) --messages-c-only --output-dir src/

# for test-cases
CC = gcc

C_FLAGS = -Wall -Wextra -Werror -Wpedantic -Wunsafe-loop-optimizations -Wold-style-definition -g -O0

DEFINES = -DPI_USE_PRINT_MSGS -DPI_STATS -DPI_DEBUG

tester: tests/tester.c src/pi-protocol.h src/pi-messages.h src/pi-messages.c src/pi-protocol.c
	$(CC) $(C_FLAGS) $(DEFINES) tests/tester.c src/pi-protocol.c src/pi-messages.c -o tester

clean : 
	$(RM) tester
	$(RM) src/pi-messages.c
	$(RM) src/pi-messages.h
	$(RM) src/pi-protocol.h
	$(RM) *.h
	$(RM) *.c
