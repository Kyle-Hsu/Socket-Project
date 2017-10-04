all:
		gcc -o client client.c 
		gcc -o edge edge.c 
		gcc -o server_or server_or.c 
		gcc -o server_and server_and.c 
 
.PHONY : server_and
server_and:
		./server_and

.PHONY : server_or
server_or:
		./server_or

.PHONY : edge
edge:
		./edge

clean:
	rm edge
	rm server_or
	rm server_and
