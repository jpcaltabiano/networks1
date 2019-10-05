all: project3

FLAGS = -g -Wall

#p3: project3 node0 node1 node2 node3
	#gcc $(FLAGS) project3 node0 node1 node2 node3 p3

project3: project3.o
	gcc $(FLAGS) project3.o -o project3

project3.o: project3.c
	gcc -c $(FLAGS) project3.c

node0: node0.o
	gcc $(FLAGS) node0.o -o node0

node0.o: node0.c
	gcc -c $(FLAGS) node0.c

node1: node1.o
	gcc $(FLAGS) node1.o -o node1

node1.o: node1.c
	gcc -c $(FLAGS) node1.c

node2: node2.o
	gcc $(FLAGS) node2.o -o node2

node2.o: node2.c
	gcc -c $(FLAGS) node2.c

node3: node3.o
	gcc $(FLAGS) node3.o -o node3

node3.o: node3.c
	gcc -c $(FLAGS) node3.c