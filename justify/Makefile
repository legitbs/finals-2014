COMMON = ../common

all:

	gcc -I $(COMMON) -s -pie -fPIE -o justify sat.c parsedimacs.c imp_graph.c decisions.c implications.c $(COMMON)/sharedfuncs.c

debug:
	gcc -I $(COMMON) -o justify.debug sat.c parsedimacs.c imp_graph.c decisions.c implications.c $(COMMON)/sharedfuncs.c -DDEBUG
