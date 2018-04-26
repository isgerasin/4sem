CC_FLAGS_SERVER=--std=c99 
CC_FLAGS_CLIENT=$(CC_FLAGS_SERVER) -lpthread

all: server client


server: server.c debug.h CalcArgs.h
	gcc -o $@ $< $(CC_FLAGS_SERVER)

client: client.c debug.h CalcArgs.h
	gcc -o $@ $< $(CC_FLAGS_CLIENT)

clean:
	rm client server
