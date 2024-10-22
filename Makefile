
test1: assignment3
	./assignment3 -l 12345  -p "happy"
	xterm nc localhost 12345 -i 0.01 < aesops_fables.txt
	
assignment3: assignment3_with_multi.c
	gcc assignment3_with_multi.c -o assignment3