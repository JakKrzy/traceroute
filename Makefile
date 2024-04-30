CFLAGS = -Wall -Wextra

traceroute: main.o Receiver.o Sender.o
	g++ -o traceroute main.o Receiver.o Sender.o $(CFLAGS) --std=c++2a

main.o: main.cpp Receiver.hpp Sender.hpp
	g++ -c main.cpp -o main.o $(CFLAGS) --std=c++2a

Receiver.o: Receiver.cpp Receiver.hpp
	g++ -c Receiver.cpp -o Receiver.o $(CFLAGS) --std=c++2a

Sender.o: Sender.cpp Sender.hpp
	g++ -c Sender.cpp -o Sender.o $(CFLAGS) --std=c++2a

clean:
	rm *.o

distclean:
	rm traceroute *.o