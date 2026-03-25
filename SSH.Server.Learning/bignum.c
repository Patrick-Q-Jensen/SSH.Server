#include "Bignum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void bn_init(BigNum* n) {
	memset(n->words, 0, sizeof(n->words));
	n->wordCount = 0;
}

void bn_copy(BigNum* dst, const BigNum* src) {
	memcpy(dst->words, src->words, sizeof(src->words));
	dst->wordCount = src->wordCount;
}

void bn_from_bytes(BigNum* n, const unsigned char* data, uint32_t len) {
	bn_init(n);

	//For each leading zero byte, we skip it and decrease the length of the data
	while (len > 0 && data[0] == 0)
	{
		data++;
		len--;
	}

	// Pack bytes into 32-bit words, least significant word first
	int wordIndex = 0;
	int remaining = (int)len;
	while (remaining > 0 && wordIndex < BIGNUM_MAX_SIZE) {
		uint32_t word = 0;
		int bytesInWord = (remaining >= 4) ? 4 : remaining;
		for (int b = 0; b < bytesInWord; b++) {
			word |= (uint32_t)data[len - 1 - (wordIndex * 4) - b] << (b * 8);
		}
		n->words[wordIndex] = word;
		wordIndex++;
		remaining -= bytesInWord;
	}
	n->wordCount = wordIndex;
}

uint32_t bn_to_bytes(const BigNum* n, unsigned char* out, uint32_t outSize) {
	int byteIndex = 0;
	for (int i = n->wordCount - 1; i >= 0; i--) {
		uint32_t word = n->words[i];
		for (int b = 3; b >= 0; b--) {
			if (byteIndex < (int)outSize) {
				out[byteIndex++] = (unsigned char)((word >> (b * 8)) & 0xFF);
			}
		}
	}
	return byteIndex;
}

void bn_from_uint32(BigNum* n, uint32_t val) {
	bn_init(n);
	if (val == 0) {
		return;
	}
	n->words[0] = val;
	n->wordCount = 1;
}

int bn_compare(const BigNum* a, const BigNum* b) {
	if (a->wordCount > b->wordCount) return 1;
	if (a->wordCount < b->wordCount) return -1;
	for (int i = a->wordCount - 1; i >= 0; i--) {
		if (a->words[i] > b->words[i]) return 1;
		if (a->words[i] < b->words[i]) return -1;
	}
	return 0;
}

bool bn_is_zero(const BigNum* n) {
	return n->wordCount == 0;
}

void bn_add(BigNum* result, const BigNum* a, const BigNum* b) {
	bn_init(result);
	uint64_t carry = 0;
	int maxWords = (a->wordCount > b->wordCount) ? a->wordCount : b->wordCount;
	for (int i = 0; i < maxWords || carry; i++) {
		uint64_t sum = carry;
		if (i < a->wordCount) sum += a->words[i];
		if (i < b->wordCount) sum += b->words[i];
		if (i < BIGNUM_MAX_SIZE) {
			result->words[i] = (uint32_t)(sum & 0xFFFFFFFF);
			result->wordCount = i + 1;
		}
		carry = sum >> 32;
	}
}

void bn_sub(BigNum* result, const BigNum* a, const BigNum* b) {
	bn_init(result);
	uint64_t borrow = 0;
	for (int i = 0; i < a->wordCount; i++)
	{
		uint64_t res = (uint64_t)a->words[i];
		uint64_t bWord = (i < b->wordCount) ? b->words[i] : 0;

		res -= borrow; // subtract any borrow from the previous word
		borrow = 0; // reset borrow for this iteration

		// if the a word is less than the corresponding b word, we need to borrow from the next word
		if (a->words[i] < b->words[i])
		{
			borrow = 1;
			res += 0x100000000; // add 2^32 to the current word to borrow
			res -= bWord; // subtract the b word from the current word
		}
		else
		{
			res -= bWord; // if no borrow is needed, just subtract the b word from the current word
		}

		result->words[i] = (uint32_t)(res & 0xFFFFFFFF); // store the result in the current word
		result->wordCount = i + 1; // update the word count
	}

	// Trim leading zero words
	while (result->wordCount > 0 && result->words[result->wordCount - 1] == 0) {
		result->wordCount--;
	}
}

void bn_mul(BigNum* result, const BigNum* a, const BigNum* b) {
	bn_init(result);
	for (int i = 0; i < a->wordCount; i++) {
		uint32_t carry = 0;
		for (int j = 0; j < b->wordCount; j++) {
			uint64_t product = (uint64_t)a->words[i] * (uint64_t)b->words[j]
				+ result->words[i + j]
				+ carry;
			result->words[i + j] = (uint32_t)(product & 0xFFFFFFFF);
			carry = (uint32_t)(product >> 32);
		}
		// Leftover carry after inner loop ends
		if (carry && (i + b->wordCount) < BIGNUM_MAX_SIZE) {
			result->words[i + b->wordCount] += carry;
		}
	}

	// Set wordCount to the maximum possible, then trim
	result->wordCount = a->wordCount + b->wordCount;
	if (result->wordCount > BIGNUM_MAX_SIZE) {
		result->wordCount = BIGNUM_MAX_SIZE;
	}
	while (result->wordCount > 0 && result->words[result->wordCount - 1] == 0) {
		result->wordCount--;
	}
}

void bn_div_mod(BigNum* quotient, BigNum* remainder, const BigNum* a, const BigNum* b) {
	bn_init(quotient);
	bn_init(remainder);


}

int bn_bit_length(const BigNum* n) {
	if (bn_is_zero(n)) return 0;
	uint32_t topWord = n->words[n->wordCount - 1];
	int rightShiftCount = 0;
	while (topWord != 0)
	{
		topWord = topWord >> 1;
		rightShiftCount++;
	}
	return (n->wordCount - 1) * 32 + rightShiftCount;
}

void bn_shift_left(BigNum* n, int bits) {
	while (bits >= 32)
	{
		n->wordCount += 1;
		for (int i = n->wordCount - 1; i > 0; i--)
		{
			n->words[i] = n->words[i - 1];
		}
		bits -= 32;
	}
	while (bits > 0)
	{
		for (int i = n->wordCount - 1; i < 0; i--)
		{
			int test = n->words[i] >> bits;
		}
	}

}

char* bn_print_hex(const BigNum* n) {
	if (bn_is_zero(n)) {
		char* str = malloc(2);
		if (str) {
			str[0] = '0';
			str[1] = '\0';
		}
		return str;
	}

	int len = n->wordCount * 8 + 1; // 8 hex chars per word + null terminator
	char* str = malloc(len);
	if (!str) return NULL;

	int written = 0;
	for (int i = n->wordCount - 1; i >= 0; i--) {
		written += snprintf(str + written, len - written, "%08x", n->words[i]);
	}
	return str;
}