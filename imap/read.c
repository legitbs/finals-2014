#include <stdio.h>
#include <unistd.h>

int readUntil(int fd, char *buf, int len, char delim) {
	char c;
	unsigned int i = 0;

	while (i < len) {
		if (read(fd, &c, 1) <= 0)
			return(-1);
		if (c == delim)
			break;
		*buf++ = c;
		i++;
	}
	*buf++ = '\0';
	return i;
}
