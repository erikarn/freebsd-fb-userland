#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/mman.h>

int
main(int argc, const char *argv[])
{
	int fd;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		err(1, "mmap /dev/fb0");
	}

	close(fd);
	exit(0);
}
