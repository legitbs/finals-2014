CFLAGS = -Wall -pie -fPIE -z execstack -I /usr/include/glib-2.0/ -I /usr/lib/x86_64-linux-gnu/glib-2.0/include

all: imap.o cmd.o read.o cmd1a.o cmd2.o base64.o
	gcc $(CFLAGS) imap.o cmd.o read.o base64.o -o imap -lglib-2.0
	gcc $(CFLAGS) imap.o cmd1a.o read.o base64.o -o imap1a -lglib-2.0
	gcc $(CFLAGS) imap.o cmd2.o read.o base64.o -o imap2 -lglib-2.0

imap.o: imap.c imap.h
	gcc $(CFLAGS) -c imap.c

cmd.o: cmd.c cmd.h imap.h
	gcc $(CFLAGS) -D VER1 -c cmd.c

cmd1a.o: cmd.c cmd.h imap.h
	gcc $(CFLAGS) -D VER1 -D VER1a -o cmd1a.o -c cmd.c

cmd2.o: cmd.c cmd.h imap.h
	gcc $(CFLAGS) -D VER2 -o cmd2.o -c cmd.c

read.o: read.c imap.h
	gcc $(CFLAGS) -c read.c

base64.o: base64.c
	gcc $(CFLAGS) -c base64.c
