all: fdinfo

fdinfo: fdinfo.c
	cc -o fdinfo fdinfo.c

clean:
	rm -f fdinfo
