target: send receive
send: send.c box.h sem.h
	gcc -o send send.c
receive: receive.c box.h sem.h
	gcc -o receive receive.c
clean:
	rm -rf send
	rm -rf receive
