
.DEFAULT_GOAL = tester
tester: tester.c pi-protocol.h pi-messages.h
	$(CC) -Wall -Wpedantic -g tester.c -o tester

clean : 
	$(RM) tester

	