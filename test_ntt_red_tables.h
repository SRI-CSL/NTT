/*
 * Tables for testing NTT using Longa&Naehrig reduction.
 */

#ifndef __TEST_NTT_TABLES_H
#define __TEST_NTT_TABLES_H

#include <stdint.h>

/*
 * Parameters
 * - for n=16: psi=1212, omega=6553
 * - for n=16, psi=1022, omega=12208
 * - for n=256: psi=1002, omega=8595
 * - for n=512: psi=1003, omega=10600
 * - for n=1024: psi=1014, omega=8209
 * - for n=2048: psi=1016, omega=12269
 *
 * All tables are scaled by inverse(3) = 8193
 */

// powers of omega in standard order
extern const int16_t shoup_red_ntt16_12289[16];
extern const int16_t shoup_red_ntt128_12289[128];
extern const int16_t shoup_red_ntt256_12289[256];
extern const int16_t shoup_red_ntt512_12289[512];
extern const int16_t shoup_red_ntt1024_12289[1024];
extern const int16_t shoup_red_ntt2048_12289[2048];

// powers of omega in bit-reverse order
extern const int16_t rev_shoup_red_ntt16_12289[16];
extern const int16_t rev_shoup_red_ntt128_12289[128];
extern const int16_t rev_shoup_red_ntt256_12289[256];
extern const int16_t rev_shoup_red_ntt512_12289[512];
extern const int16_t rev_shoup_red_ntt1024_12289[1024];
extern const int16_t rev_shoup_red_ntt2048_12289[2048];

// powers of omega and psi in standard order
extern const int16_t shoup_red_scaled_ntt16_12289[16];
extern const int16_t shoup_red_scaled_ntt128_12289[128];
extern const int16_t shoup_red_scaled_ntt256_12289[256];
extern const int16_t shoup_red_scaled_ntt512_12289[512];
extern const int16_t shoup_red_scaled_ntt1024_12289[1024];
extern const int16_t shoup_red_scaled_ntt2048_12289[2048];

// powers of omega and psi in bit-reverse order
extern const int16_t rev_shoup_red_scaled_ntt16_12289[16];
extern const int16_t rev_shoup_red_scaled_ntt128_12289[128];
extern const int16_t rev_shoup_red_scaled_ntt256_12289[256];
extern const int16_t rev_shoup_red_scaled_ntt512_12289[512];
extern const int16_t rev_shoup_red_scaled_ntt1024_12289[1024];
extern const int16_t rev_shoup_red_scaled_ntt2048_12289[2048];

/*
 * Same tables but with coefficients in the interval [-6144,+6144]
 */
// powers of omega in standard order
extern const int16_t shoup_sred_ntt16_12289[16];
extern const int16_t shoup_sred_ntt128_12289[128];
extern const int16_t shoup_sred_ntt256_12289[256];
extern const int16_t shoup_sred_ntt512_12289[512];
extern const int16_t shoup_sred_ntt1024_12289[1024];
extern const int16_t shoup_sred_ntt2048_12289[2048];

// powers of omega in bit-reverse order
extern const int16_t rev_shoup_sred_ntt16_12289[16];
extern const int16_t rev_shoup_sred_ntt128_12289[128];
extern const int16_t rev_shoup_sred_ntt256_12289[256];
extern const int16_t rev_shoup_sred_ntt512_12289[512];
extern const int16_t rev_shoup_sred_ntt1024_12289[1024];
extern const int16_t rev_shoup_sred_ntt2048_12289[2048];

// powers of omega and psi in standard order
extern const int16_t shoup_sred_scaled_ntt16_12289[16];
extern const int16_t shoup_sred_scaled_ntt128_12289[128];
extern const int16_t shoup_sred_scaled_ntt256_12289[256];
extern const int16_t shoup_sred_scaled_ntt512_12289[512];
extern const int16_t shoup_sred_scaled_ntt1024_12289[1024];
extern const int16_t shoup_sred_scaled_ntt2048_12289[2048];

// powers of omega and psi in bit-reverse order
extern const int16_t rev_shoup_sred_scaled_ntt16_12289[16];
extern const int16_t rev_shoup_sred_scaled_ntt128_12289[128];
extern const int16_t rev_shoup_sred_scaled_ntt256_12289[256];
extern const int16_t rev_shoup_sred_scaled_ntt512_12289[512];
extern const int16_t rev_shoup_sred_scaled_ntt1024_12289[1024];
extern const int16_t rev_shoup_sred_scaled_ntt2048_12289[2048];


#endif /* __TEST_NTT_TABLES_H */
