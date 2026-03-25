#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Bignum.h"

static int tests_passed = 0;
static int tests_failed = 0;

BigNum a;
BigNum b;
BigNum result;

#define TEST(name) void name(void)
#define RUN_TEST(name) do { \
    printf("Running %s... ", #name); \
    name(); \
    tests_passed++; \
    printf("PASSED\n"); \
} while(0)

// ---- bn_from_uint32 / bn_is_zero ----

TEST(test_from_uint32_zero) {
    BigNum n;
    bn_from_uint32(&n, 0);
    assert(bn_is_zero(&n));
    assert(n.wordCount == 0);
}

TEST(test_from_uint32_nonzero) {
    BigNum n;
    bn_from_uint32(&n, 42);
    assert(!bn_is_zero(&n));
    assert(n.wordCount == 1);
    assert(n.words[0] == 42);
}

// ---- bn_compare ----

TEST(test_compare_equal) {
    bn_init(&a);
	bn_init(&b);
    bn_from_uint32(&a, 100);
    bn_from_uint32(&b, 100);
    assert(bn_compare(&a, &b) == 0);
}

TEST(test_compare_greater) {
    bn_init(&a);
    bn_init(&b);
    bn_from_uint32(&a, 200);
    bn_from_uint32(&b, 100);
    assert(bn_compare(&a, &b) > 0);
}

TEST(test_compare_less) {
    bn_init(&a);
    bn_init(&b);
    bn_from_uint32(&a, 50);
    bn_from_uint32(&b, 100);
    assert(bn_compare(&a, &b) < 0);
}

// ---- bn_add ----

TEST(test_add_simple) {
    bn_init(&a);
    bn_init(&b);
    bn_init(&result);
    bn_from_uint32(&a, 100);
    bn_from_uint32(&b, 200);
    bn_add(&result, &a, &b);
    assert(result.words[0] == 300);
    assert(result.wordCount == 1);
}

TEST(test_add_with_carry) {
    bn_init(&a);
    bn_init(&b);
    bn_init(&result);
    bn_from_uint32(&a, 0xFFFFFFFF);
    bn_from_uint32(&b, 1);
    bn_add(&result, &a, &b);
    assert(result.words[0] == 0x00000000);
    assert(result.words[1] == 0x00000001);
    assert(result.wordCount == 2);
}

TEST(test_add_large_numbers) {
    bn_init(&a);
    bn_init(&b);
    bn_init(&result);
    bn_from_uint32(&a, 0xFFFFFFFF);
    bn_from_uint32(&b, 0xFFFFFFFF);
    bn_add(&result, &a, &b);
    assert(result.words[0] == 0xFFFFFFFE);
    assert(result.words[1] == 0x00000001);
    assert(result.wordCount == 2);
	const char* hex = bn_print_hex(&result);// Should print 1FFFFFFFE
    assert(strcmp("00000001fffffffe", hex) == 0);
}

// ---- bn_sub ----

TEST(test_sub_simple) {
    BigNum a, b, result;
    bn_from_uint32(&a, 300);
    bn_from_uint32(&b, 100);
    bn_sub(&result, &a, &b);
    assert(result.words[0] == 200);
    assert(result.wordCount == 1);
}

TEST(test_sub_equal) {
    BigNum a, b, result;
    bn_from_uint32(&a, 42);
    bn_from_uint32(&b, 42);
    bn_sub(&result, &a, &b);
    assert(bn_is_zero(&result));
}

// ---- bn_mul ----

TEST(test_mul_simple) {
    // 3 * 7 = 21
    bn_from_uint32(&a, 3);
    bn_from_uint32(&b, 7);
    bn_mul(&result, &a, &b);
    assert(result.wordCount == 1);
    assert(result.words[0] == 21);
}

TEST(test_mul_by_zero) {
    bn_from_uint32(&a, 12345);
    bn_from_uint32(&b, 0);
    bn_mul(&result, &a, &b);
    assert(bn_is_zero(&result));
}

TEST(test_mul_by_one) {
    bn_from_uint32(&a, 0xDEADBEEF);
    bn_from_uint32(&b, 1);
    bn_mul(&result, &a, &b);
    assert(result.wordCount == 1);
    assert(result.words[0] == 0xDEADBEEF);
}

TEST(test_mul_overflow_to_two_words) {
    // 0xFFFFFFFF * 0xFFFFFFFF = 0xFFFFFFFE00000001
    bn_from_uint32(&a, 0xFFFFFFFF);
    bn_from_uint32(&b, 0xFFFFFFFF);
    bn_mul(&result, &a, &b);
    assert(result.wordCount == 2);
    assert(result.words[0] == 0x00000001);
    assert(result.words[1] == 0xFFFFFFFE);
}

TEST(test_mul_known_value) {
    // 0x10000 * 0x10000 = 0x100000000 (crosses into second word)
    bn_from_uint32(&a, 0x10000);
    bn_from_uint32(&b, 0x10000);
    bn_mul(&result, &a, &b);
    assert(result.wordCount == 2);
    assert(result.words[0] == 0x00000000);
    assert(result.words[1] == 0x00000001);
}

TEST(test_mul_multi_word) {
    // (2^32 + 1) * 2 = 2^33 + 2 = 0x0000000200000002
    unsigned char data[] = { 0x01, 0x00, 0x00, 0x00, 0x01 }; // 0x100000001
    bn_from_bytes(&a, data, sizeof(data));
    bn_from_uint32(&b, 2);
    bn_mul(&result, &a, &b);
    assert(result.wordCount == 2);
    assert(result.words[0] == 0x00000002);
    assert(result.words[1] == 0x00000002);
}

// ---- bn_bit_length ----

TEST(test_bit_length_zero) {
    bn_from_uint32(&a, 0);
    assert(bn_bit_length(&a) == 0);
}

TEST(test_bit_length_one) {
    // 0x01 = 1 bit
    bn_from_uint32(&a, 1);
    assert(bn_bit_length(&a) == 1);
}

TEST(test_bit_length_byte_boundary) {
    // 0xFF = 8 bits
    bn_from_uint32(&a, 0xFF);
    assert(bn_bit_length(&a) == 8);
}

TEST(test_bit_length_just_over_byte) {
    // 0x100 = 9 bits
    bn_from_uint32(&a, 0x100);
    assert(bn_bit_length(&a) == 9);
}

TEST(test_bit_length_full_word) {
    // 0x80000000 = 32 bits
    bn_from_uint32(&a, 0x80000000);
    assert(bn_bit_length(&a) == 32);
}

TEST(test_bit_length_max_word) {
    // 0xFFFFFFFF = 32 bits
    bn_from_uint32(&a, 0xFFFFFFFF);
    assert(bn_bit_length(&a) == 32);
}

TEST(test_bit_length_two_words) {
    // 0x100000000 = 33 bits (just crosses into second word)
    unsigned char data[] = { 0x01, 0x00, 0x00, 0x00, 0x00 };
    bn_from_bytes(&a, data, sizeof(data));
    assert(bn_bit_length(&a) == 33);
}

TEST(test_bit_length_two_words_large) {
    // 0x0001FFFFFFFE → top word is 0x0001 = 1 bit + 32 = 33 bits
    unsigned char data[] = { 0x01, 0xFF, 0xFF, 0xFF, 0xFE };
    bn_from_bytes(&a, data, sizeof(data));
    assert(bn_bit_length(&a) == 33);
}

// ---- bn_from_bytes / bn_to_bytes ----

TEST(test_from_bytes_single_word) {
    // 0xDEADBEEF in big-endian bytes
    unsigned char data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    bn_from_bytes(&a, data, sizeof(data));
    assert(a.wordCount == 1);
    assert(a.words[0] == 0xDEADBEEF);
}

TEST(test_from_bytes_two_words) {
    // 0x0100000002 in big-endian bytes
    unsigned char data[] = { 0x01, 0x00, 0x00, 0x00, 0x02 };
    bn_from_bytes(&a, data, sizeof(data));
    assert(a.wordCount == 2);
    assert(a.words[0] == 0x00000002);
    assert(a.words[1] == 0x00000001);
}

TEST(test_from_bytes_skips_leading_zeros) {
    // Leading 0x00 (mpint sign padding) followed by 0xFF000001
    unsigned char data[] = { 0x00, 0xFF, 0x00, 0x00, 0x01 };
    bn_from_bytes(&a, data, sizeof(data));
    assert(a.wordCount == 1);
    assert(a.words[0] == 0xFF000001);
}

TEST(test_from_bytes_all_zeros) {
    unsigned char data[] = { 0x00, 0x00, 0x00 };
    bn_from_bytes(&a, data, sizeof(data));
    assert(bn_is_zero(&a));
}

TEST(test_to_bytes_roundtrip) {
    // Start with known big-endian bytes, convert to BigNum, convert back
    unsigned char input[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    unsigned char output[8];
    bn_from_bytes(&a, input, sizeof(input));
    uint32_t len = bn_to_bytes(&a, output, sizeof(output));
    assert(len == 8);
    assert(memcmp(input, output, 8) == 0);
}


// ---- Entry point ----

int main(void) {
    printf("=== BigNum Tests ===\n\n");

    RUN_TEST(test_from_uint32_zero);
    RUN_TEST(test_from_uint32_nonzero);
    RUN_TEST(test_compare_equal);
    RUN_TEST(test_compare_greater);
    RUN_TEST(test_compare_less);
    RUN_TEST(test_add_simple);
    RUN_TEST(test_add_with_carry);
    RUN_TEST(test_add_large_numbers);
    RUN_TEST(test_sub_simple);
    RUN_TEST(test_sub_equal);
    RUN_TEST(test_mul_simple);
    RUN_TEST(test_mul_by_zero);
    RUN_TEST(test_mul_by_one);
    RUN_TEST(test_mul_overflow_to_two_words);
    RUN_TEST(test_mul_known_value);
    RUN_TEST(test_mul_multi_word);
    RUN_TEST(test_bit_length_zero);
    RUN_TEST(test_bit_length_one);
    RUN_TEST(test_bit_length_byte_boundary);
    RUN_TEST(test_bit_length_just_over_byte);
    RUN_TEST(test_bit_length_full_word);
    RUN_TEST(test_bit_length_max_word);
    RUN_TEST(test_bit_length_two_words);
    RUN_TEST(test_bit_length_two_words_large);
    RUN_TEST(test_from_bytes_single_word);
    RUN_TEST(test_from_bytes_two_words);
    RUN_TEST(test_from_bytes_skips_leading_zeros);
    RUN_TEST(test_from_bytes_all_zeros);
    RUN_TEST(test_to_bytes_roundtrip);

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}