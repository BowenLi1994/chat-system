main: 
	g++ -std=c++11 -g -pthread -o client messager_client.cpp
	g++ -std=c++11 -g -pthread -o server messager_server.cpp
clean:
	rm client
	rm server
