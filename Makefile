build:
	g++ -g -Wall server.cpp -o server
	g++ -g -Wall subscriber.cpp -o subscriber

clean:
	rm -f server subscriber
