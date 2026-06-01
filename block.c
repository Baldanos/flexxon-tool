#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "block.h"

#define BLOCK_SIZE 512

/* ---------------------------------------------------- */

int write_single_block(int fd, uint32_t lba, const uint8_t *buf)
{
	void *aligned_buf = NULL;

	// Allocate aligned buffer for O_DIRECT
	if (posix_memalign(&aligned_buf, BLOCK_SIZE, BLOCK_SIZE) != 0) {
	perror("posix_memalign");
	return -1;
	}

	memcpy(aligned_buf, (void *)buf, BLOCK_SIZE);

	off_t offset = (off_t)lba * BLOCK_SIZE;

	ssize_t written = pwrite(fd, aligned_buf, BLOCK_SIZE, offset);
	if (written != BLOCK_SIZE) {
	perror("pwrite");
	free(aligned_buf);
	return -1;
	}

	fsync(fd);
	free(aligned_buf);

	return 0;
}

int read_single_block(int fd, uint32_t lba, uint8_t *buf)
{
	void *aligned_buf = NULL;

	// Allocate aligned buffer for O_DIRECT
	if (posix_memalign(&aligned_buf, BLOCK_SIZE, BLOCK_SIZE) != 0) {
	perror("posix_memalign");
	return -1;
	}

	off_t offset = (off_t)lba * BLOCK_SIZE;

	ssize_t r = pread(fd, aligned_buf, BLOCK_SIZE, offset);
	if (r != BLOCK_SIZE) {
		perror("pread");
		close(fd);
		free(aligned_buf);
	return -1;
	}

	memcpy((void *)buf, aligned_buf, BLOCK_SIZE);

	fsync(fd);
	free(aligned_buf);

	return 0;
}


