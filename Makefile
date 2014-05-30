CC := $(CROSS_COMPILE)gcc
LIBS := -lpthread

all:
	mkdir -p clientFiles
	mkdir -p serverFiles
	$(CC) -D PTHREAD -o client client.c $(LIBS)
	$(CC) -D PTHREAD -o server server.c $(LIBS)
	mv client clientFiles
	mv server serverFiles
	cp -p ServerDoc.in serverFiles
clean:
	rm -r -f ./clientFiles
	rm -r -f ./serverFiles
