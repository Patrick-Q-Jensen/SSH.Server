#ifndef BIGNUM_H
#define BIGNUM_H

#include <stdint.h>
#include <stdbool.h>

#define BIGNUM_MAX_SIZE 64

typedef struct Bignum {
	uint32_t words[BIGNUM_MAX_SIZE];
	int wordCount;
} BigNum;

//Lifecycle
void bn_init(BigNum* n);
void bn_copy(BigNum* dst, const BigNum* src);

// Conversion
void bn_from_bytes(BigNum* n, const unsigned char* data, uint32_t len);
uint32_t bn_to_bytes(const BigNum* n, unsigned char* out, uint32_t outSize);
void bn_from_uint32(BigNum* n, uint32_t val);

// Comparison
int bn_compare(const BigNum* a, const BigNum* b);
bool bn_is_zero(const BigNum* n);

// Arithmetic
void bn_add(BigNum* result, const BigNum* a, const BigNum* b);
void bn_sub(BigNum* result, const BigNum* a, const BigNum* b); // assumes a >= b
void bn_mul(BigNum* result, const BigNum* a, const BigNum* b);
void bn_div_mod(BigNum* quotient, BigNum* remainder, const BigNum* a, const BigNum* b);

// Modular arithmetic
void bn_mod(BigNum* result, const BigNum* a, const BigNum* m);
void bn_mod_exp(BigNum* result, const BigNum* base, const BigNum* exp, const BigNum* mod);

// Bit operations
void bn_shift_left(BigNum* n, int bits);
void bn_shift_right(BigNum* n, int bits);
int bn_bit_length(const BigNum* n);

// Debug
char* bn_print_hex(const BigNum* n);

#endif // !BIGNUM_H

