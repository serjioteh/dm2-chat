all:
	g++ -std=c++11 ChatClient.cpp -o client
	g++ -std=c++11 Chat.cpp Client.cpp Server.cpp -o server
clean:
	rm -f client server
