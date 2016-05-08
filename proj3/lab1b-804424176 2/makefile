all:
	@gcc -pthread -lmcrypt -ldl server.c -o server
	@chmod +x server
	@gcc -pthread -lmcrypt -ldl client.c -o client
	@chmod +x client


server:
	@gcc -pthread -lmcrypt -ldl server.c -o server
	@chmod +x server

client:
	@gcc -pthread -lmcrypt -ldl client.c -o client
	@chmod +x client


clean:
	@-rm -f client
	@-rm -f server



dist:
	tar -cvf lab1b-804424176.tar.gz server.c client.c my.key makefile  README
