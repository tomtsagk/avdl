#include "file_op.h"
#include <unistd.h>
#include <fcntl.h>

int file_copy(const char *src, const char *dest, int append) {
	int s = open(src, O_RDONLY);
	int d = open(dest, O_WRONLY);

	char buffer[1024];

	int i = 0;
	ssize_t nread;
	while (nread = read(s, buffer, 1), nread > 0) {
		//fscanf(s, "%1023c", buffer);
		//buffer[1023] = '\0';
	//	printf("%s\n", buffer);
		write(d, buffer, nread);
		printf("i: %d\n", i);
		i++;
	}


	close(s);
	close(d);
}
