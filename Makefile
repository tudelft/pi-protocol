
.DEFAULT_GOAL = tester
tester: tester.c pi-protocol.h pi-messages.h
	$(CC) tester.c -o tester

	