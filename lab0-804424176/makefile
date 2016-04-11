lab0 : lab0.c
	@gcc -g lab0.c -o lab0
	@chmod +x lab0



clean:
	@-rm -f lab0.o
	@-rm -f lab0

check:
	@gcc -g lab0.c -o lab0
	@echo "something" > 1.txt
	@./lab0 --input=1.txt --output=2.txt

	      
	

	@cmp --quiet 1.txt 2.txt && echo "it works" || echo "it doesn't work"
	

	@-rm -f lab0.o
	@-rm -f lab0
	@-rm -f 1.txt
	@-rm -f 2.txt

dist:
	tar -cvf lab0-804424176.tar.gz lab0.c makefile screen1.jpg screen2.jpg README





