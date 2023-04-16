all: server client clientPdf serverPdf

server: server.cpp server.h tands.o
	g++ server.cpp tands.o -o server -O

client: client.cpp client.h tands.o
	g++ client.cpp tands.o -o client -O

tands.o: tands.cpp tands.h
	g++ -c tands.cpp -O

serverPdf: server.man
	groff -Tpdf -man server.man >server.pdf

clientPdf: client.man
	groff -Tpdf -man client.man >client.pdf

clean:
	rm server client *Output *.o *.txt *.pdf