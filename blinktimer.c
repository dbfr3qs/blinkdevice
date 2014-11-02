#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/blinkdevice"

int main() {
	int i, fd;
	char ch;
	char *write_buf;

	fd = open(DEVICE, O_RDWR);

	if(fd == -1) {
		printf("file %s either does not exist of is locked by another process\n", DEVICE);
		exit(-1);
	}

	while(1) {
		write_buf = "HIGH";
		write(fd, write_buf, sizeof(write_buf));
		sleep(1);
		write_buf = "LOW";
		write(fd, write_buf, sizeof(write_buf));
		sleep(1);
	}
	return 0;
}
