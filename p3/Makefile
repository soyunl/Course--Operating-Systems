all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc diskinfo.c -o diskinfo

disklist: disklist.c linkedlist.c
	gcc disklist.c linkedlist.c -o disklist

diskget: diskget.c
	gcc diskget.c -o diskget

diskput: diskput.c linkedlist.c
	gcc diskput.c linkedlist.c -o diskput

.PHONY clean:
clean:
	-rm diskinfo disklist diskget diskput

