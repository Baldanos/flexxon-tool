#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#include "block.h"
#include "xmask_cmd.h"

int fd;
uint8_t buf[512];
uint8_t P1[512];

void enter_cmd2()
{
	write_single_block(fd, 0, enter_cmd2_buf);
	read_single_block(fd, 5, P1);
	read_single_block(fd, 5, P1);
}

void leave_cmd2()
{
	write_single_block(fd, 0, leave_cmd2_buf);
}


/* Placeholder lock/unlock handlers */
void do_lock(int fd)
{
	printf("LOCK\r\n");
	enter_cmd2();
	encode_response(P1, 512, (uint8_t *)"\x00\x00\x00\x00", 4, buf);
	write_single_block(fd, 9, buf);
	leave_cmd2();
}

void do_unlock(int fd)
{
	printf("UNLOCK\r\n");
	enter_cmd2();
	encode_response(P1, 512, (uint8_t *)"\xFF\xFF\xFF\xFF", 4, buf);
	write_single_block(fd, 9, buf);
	leave_cmd2();
}

int main(int argc, char **argv)
{
	const char *device = NULL;
	int do_lock_flag = 0;
	int do_unlock_flag = 0;

	static struct option long_opts[] = {
		{"device", required_argument, 0, 'd'},
		{"lock",   no_argument,       0, 'l'},
		{"unlock", no_argument,       0, 'u'},
		{0, 0, 0, 0}
	};

	int opt;
	int opt_index = 0;

	while ((opt = getopt_long(argc, argv, "d:lu", long_opts, &opt_index)) != -1) {
		switch (opt) {
		case 'd':
			device = optarg;
			break;
		case 'l':
			do_lock_flag = 1;
			break;
		case 'u':
			do_unlock_flag = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s --device <dev> [--lock | --unlock]\n", argv[0]);
			return 1;
		}
	}

	/* Validate arguments */
	if (!device) {
		fprintf(stderr, "Error: --device is required\n");
		return 1;
	}

	if (do_lock_flag && do_unlock_flag) {
		fprintf(stderr, "Error: --lock and --unlock are mutually exclusive\n");
		return 1;
	}

	if (!do_lock_flag && !do_unlock_flag) {
		fprintf(stderr, "Error: either --lock or --unlock must be specified\n");
		return 1;
	}

	/* Open device */
	fd = open(device, O_RDWR | O_DIRECT | O_SYNC);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	/* Execute requested operation */
	if (do_lock_flag)
		do_lock(fd);
	else if (do_unlock_flag)
		do_unlock(fd);

	close(fd);
	return 0;
}

