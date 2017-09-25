all: client.cpp edge.cpp server_and.cpp server_or.cpp
	g++ client.cpp -o client
	g++ edge.cpp -o edge
	g++ server_and.cpp -o and
	g++ server_or.cpp -o or


client:
	./client jobs.txt

server_and:
	./and

server_or:
	./or

edge:
	./edge
	
killall:
	killall edge
	killall and
	killall or
