LDLIBS = -lpthread

server : server client.o filebuffer.o handler.o logging.o main.o time.o packet.o
	g++ -g -o server client.o filebuffer.o handler.o logging.o main.o time.o packet.o $(LDLIBS)

client.o : main.h client.cpp 
	g++ -c -g client.cpp 

filebuffer.o : main.h filebuffer.cpp 
	g++ -c -g filebuffer.cpp 

handler.o : main.h handler.cpp 
	g++ -c -g handler.cpp

logging.o : main.h logging.cpp 
	g++ -c -g logging.cpp 

main.o : main.h main.cpp 
	g++ -c -g main.cpp

time.o : main.h time.cpp 
	g++ -c -g time.cpp 

packet.o : main.h packet.cpp 
	g++ -c -g packet.cpp

clean:
	rm *.o server 