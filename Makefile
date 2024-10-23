
creating_server: assignment3
	./assignment3 -l 1234  -p "the"

client1: 
	nc localhost 1234 -i 2 < aesops_fables.txt

client2: 
	nc localhost 1234 -i 2 < metamorphosis_kafka.txt

client3: 
	nc localhost 1234 -i 2 < rome_and_juliet.txt
	
assignment3: assignment3.c
	gcc assignment3.c -o assignment3