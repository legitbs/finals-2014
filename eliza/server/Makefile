CC=gcc
#CCFLAGS=-g3 -Wall -w -pedantic -fmessage-length=0 -fno-stack-protector -D_FORTIFY_SOURCE=0 -pie -fPIE
#CCFLAGS=-O3 -pg -D_POSIX_C_SOURCE=199309L -std=c99 -m32 -Wall
CCFLAGS=-fno-toplevel-reorder -O3 -D_POSIX_C_SOURCE=199309L -std=c99 -fvisibility=hidden -m32 -g3 -Wall -w -pedantic -fmessage-length=0 -fno-stack-protector -pie -fPIE
#CCFLAGS=-O3 -D_POSIX_C_SOURCE=199309L -std=c99 -m32 -g3 -Wall -w -pedantic -fmessage-length=0 -fno-stack-protector -pie -fPIE

O_FILES = eliza.o

eliza : $(O_FILES)
	$(CC) $(CCFLAGS) -o eliza $(O_FILES)

# dependency stuff, see: http://www.cs.berkeley.edu/~smcpeak/autodepend/autodepend.html
# pull in dependency info for *existing* .o files
-include $(O_FILES:.o=.d)

.SUFFIXES : .o .c

.c.o :  
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f eliza *.o *.d
