#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"

static void bytes2bits(const uint8_t *data, size_t len, char *out, size_t *out_len)
{
	size_t pos = 0;
	for (size_t i = 0; i < len; i++) {
		uint8_t v = data[i];
		for (int bit = 7; bit >= 0; bit--) {
			out[pos++] = ((v >> bit) & 1) ? '1' : '0';
		}
	}
	if (out_len) {
		*out_len = pos;
	}
}

static void bytes2twistedbits(const uint8_t *data, size_t len, char *out, size_t *out_len)
{
	size_t pos = 0;
	for (size_t i = 0; i < len; i++) {
		uint8_t v = data[i];
		for (int bit = 0; bit <= 7; bit++) {
			out[pos++] = ((v >> bit) & 1) ? '1' : '0';
		}
	}
	if (out_len) {
		*out_len = pos;
	}
}

/* cksum(payload: bytearray) */
void cksum(uint8_t payload[512])
{
	uint32_t S = 0;
	for (int i = 0; i < 256; i++) {
		uint16_t word = (uint16_t)((payload[(2 * i)] << 8) | payload[(2 * i) + 1]);
		S += (uint32_t)(word & 0xFFFFu);
	}
	S &= 0xFFFFu;
	payload[510] = (uint8_t)(S >> 8);
	payload[511] = (uint8_t)(S & 0xFF);
}

/*
 * pkt: input packet bytes
 * pkt_len: length of pkt (must be large enough for accesses)
 * password: password bytes (can be NULL to use default "7890")
 * pwd_len: length of password (ignored if password == NULL)
 * payload: output buffer of 512 bytes (filled by this function)
 */
void encode_response(const uint8_t *pkt,
                     size_t pkt_len,
                     const uint8_t *password,
                     size_t pwd_len,
                     uint8_t payload[512])
{
	(void)pkt_len; /* assuming caller ensures pkt is large enough */

	/* Prepare password: default "7890" and pad to 0x40 bytes with zeros */
	uint8_t pwd_buf[0x40];

	memset(pwd_buf, 0, sizeof(pwd_buf));
	if (pwd_len > sizeof(pwd_buf)) {
		memcpy(pwd_buf, password, sizeof(pwd_buf));
	} else {
		memcpy(pwd_buf, password, pwd_len);
	}

	/* a = int.from_bytes(pkt[0x39:0x3d], "big") */
	uint32_t a = ((uint32_t)pkt[0x39] << 24) |
	             ((uint32_t)pkt[0x3A] << 16) |
	             ((uint32_t)pkt[0x3B] << 8)  |
	             (uint32_t)pkt[0x3C];

	/* b = int.from_bytes(pkt[0x41:0x45], "big") */
	uint32_t b = ((uint32_t)pkt[0x41] << 24) |
	             ((uint32_t)pkt[0x42] << 16) |
	             ((uint32_t)pkt[0x43] << 8)  |
	             (uint32_t)pkt[0x44];

	uint32_t off = (uint32_t)((a + b) & 0xFF);
	off += 0xB8;

	const uint8_t *key = &pkt[off]; /* 8 bytes */
	/* Initialize payload to zero (Python's bytearray(512)) */
	memset(payload, 0, 512);


	uint8_t bit_pos = key[1] & 0x7;
	size_t I = key[0];

	/* Build pwdbits (64 bytes -> 512 bits) */
	char pwdbits[64 * 8];
	size_t pwdbits_len = 0;
	bytes2bits(pwd_buf, 0x40, pwdbits, &pwdbits_len); /* pwdbits_len == 512 */

	/* Build twisted bits of key (8 bytes -> 64 bits) */
	char keybits[8 * 8];
	size_t keybits_len = 0;
	bytes2twistedbits(key, 8, keybits, &keybits_len); /* keybits_len == 64 */

	size_t PI = 0;
	for (size_t kb = 0; kb < keybits_len; kb++) {
		char bit = keybits[kb];
		if (bit == '0') {
			for (int i = 0; i < 4; i++) {
				if (pwdbits[PI + i] == '1') {
					payload[I] &= (uint8_t)(~(1u << bit_pos) & 0xFFu);
					I++;
				} else {
					payload[I] |= (uint8_t)(1u << bit_pos);
					I++;
				}
			}
			PI += 4;
		} else {
			for (int i = 0; i < 4; i++) {
				if (pwdbits[PI + i] == '1') {
					payload[I] |= (uint8_t)(1u << bit_pos);
					I++;
				} else {
					payload[I] &= (uint8_t)(~(1u << bit_pos) & 0xFFu);
					I++;
				}
			}
			PI += 4;
		}
		if (PI >= 256) {
			break;
		}
	}
	cksum(payload);
}

