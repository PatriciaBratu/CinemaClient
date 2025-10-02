CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c parson.c admin.c user.c movies.c collection.c
	$(CC) -o client client.c requests.c helpers.c parson.c admin.c user.c movies.c collection.c -Wall

run: client
	./client

clean:
	rm -f *.o client
