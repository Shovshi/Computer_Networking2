all: PartA PartB

PartA: ping

PartB: better_ping watchdog
	
ping: 
	gcc -Wall ping.c -o PartA
	
better_ping:
	gcc -Wall better_ping.c -o PartB

watchdog: watchdog.c
	gcc -Wall watchdog.c -o watchdog

clean:
	rm -f *.o PartA PartB watchdog