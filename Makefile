
creating_server_sp1: assignment3 # search pattern 1
	./assignment3 -l 1234  -p "happy"

creating_server_sp2: assignment3 # search pattern 2
	./assignment3 -l 1234  -p "when"

creating_server_sp3: assignment3 # search pattern 3
	./assignment3 -l 1234  -p "bird"

creating_server_sp4: assignment3 # search pattern 4
	./assignment3 -l 1234  -p "cloud"

client1: 
	nc localhost 1234 -i 2 < aesops_fables.txt

client2: 
	nc localhost 1234 -i 2 < metamorphosis_kafka.txt

client3: 
	nc localhost 1234 -i 2 < rome_and_juliet.txt
	
assignment3: assignment3.c
	gcc assignment3.c -o assignment3