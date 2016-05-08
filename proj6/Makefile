CC = gcc

# (default) ... build the lab2a executable.


lab2b: lab2b.c SortedList.h SortedList.c
	@$(CC) -pthread lab2b.c SortedList.h SortedList.c -o $@



# dist ... build the distribution tarball.
dist: lab2b-304451718.tar.gz
submission_files = Makefile \
  lab2b.c README graph1.png \
  graph2.png SortedList.h SortedList.c
lab2b-304451718.tar.gz: $(submission_files)
	@tar -czf $@ $(submission_files)
	


