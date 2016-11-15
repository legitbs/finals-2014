CC=gcc
CFLAGS=-Wall -lm -fno-stack-protector -lz
COMMON =../common/
TINY=TinyMT-src-1.0.3/tinymt/
SHARED=../common/sharedfuncs.c
EXECUTABLE=wdub
SOURCES1=wdub.c post.c connect.c options.c patch.c trace.c headeradds.c string_functions.c itoa.c b64.c $(TINY)tinymt32.c $(SHARED)
SOURCES=wdub.c post.c connect.c eval.c options.c patch.c trace.c headeradds.c string_functions.c itoa.c b64.c yodog.c $(TINY)tinymt32.c $(SHARED)

all: xinetd-v1 xinetd-v2 debug-v1 debug-v2

xinetd-v1:
	$(CC) $(SOURCES1) $(CFLAGS) -DVONE -DXINETD -D$(shell uname -p)  -s -I $(TINY) -I $(COMMON) -o wdub-xinetd-v1

xinetd-v2:
	$(CC) $(SOURCES) $(CFLAGS)  -DXINETD -D$(shell uname -p)  -s -I $(TINY) -I $(COMMON) -o wdub-xinetd-v2

debug-v1:
	$(CC) $(SOURCES1) $(CFLAGS) -DVONE -D$(shell uname -p) -DDEBUG -I$(TINY) -I $(COMMON) -o wdub-debug-v1

debug-v2:
	$(CC) $(SOURCES) $(CFLAGS) -D$(shell uname -p) -DDEBUG -I$(TINY) -I $(COMMON) -o wdub-debug-v2

myscript:
	$(CC) -DDEBUG myscript.c yodog.c itoa.c -I$(COMMON) -I$(TINY) $(CFLAGS) -o mys
