all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc -Wall -pthread -g -o diskinfo diskinfo.c

disklist: disklist.c
	gcc -Wall -pthread -g -o disklist disklist.c

diskget: diskget.c
	gcc -Wall -pthread -g -o diskget diskget.c

diskput: diskput.c
	gcc -Wall -pthread -g -o diskput diskput.c

clean:
	rm -f *.o
	rm -f diskinfo disklist diskget diskput
