#include <stdint.h>

int read_single_block(int fd, uint32_t lba, uint8_t *buf);
int write_single_block(int fd, uint32_t lba, const uint8_t *buf);

