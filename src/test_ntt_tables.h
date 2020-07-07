/*
 * Tables for testing the NTT functions
 */

#ifndef __TEST_NTT_TABLES_H
#define __TEST_NTT_TABLES_H

#include <stdint.h>

/*
 * Powers of psi:
 * - for n=16: psi=1212, omega=6553
 * - for n=256: psi=1002, omega=8595
 * - for n=512: psi=1003, omega=10600
 * - for n=1024: psi=1014, omega=8209
 */
extern const uint16_t psi_powers_ntt16_12289[16];
extern const uint16_t psi_powers_ntt256_12289[256];
extern const uint16_t psi_powers_ntt512_12289[512];
extern const uint16_t psi_powers_ntt1024_12289[1024];

/*
 * Powers of omega in Shoup-style format
 * - use the same parameters psi/omega as above
 */
extern const uint16_t shoup_ntt16_12289[16];
extern const uint16_t shoup_ntt256_12289[256];
extern const uint16_t shoup_ntt512_12289[512];
extern const uint16_t shoup_ntt1024_12289[1024];

/*
 * Scaled tables in Shoup-style format:
 * - powers of omega multiplied by powers of psi
 */
extern const uint16_t shoup_scaled_ntt16_12289[16];
extern const uint16_t shoup_scaled_ntt256_12289[256];
extern const uint16_t shoup_scaled_ntt512_12289[512];
extern const uint16_t shoup_scaled_ntt1024_12289[1024];

/*
 * Powers of omega in bitreverse/Shoup-style format
 * - use the same parameters psi/omega as above
 */
extern const uint16_t rev_shoup_ntt16_12289[16];
extern const uint16_t rev_shoup_ntt256_12289[256];
extern const uint16_t rev_shoup_ntt512_12289[512];
extern const uint16_t rev_shoup_ntt1024_12289[1024];

/*
 * Powers of omega and spi in bitreverse/Shoup-style format
 * - use the same parameters psi/omega as above
 */
extern const uint16_t rev_shoup_scaled_ntt16_12289[16];
extern const uint16_t rev_shoup_scaled_ntt256_12289[256];
extern const uint16_t rev_shoup_scaled_ntt512_12289[512];
extern const uint16_t rev_shoup_scaled_ntt1024_12289[1024];


#endif /* __TEST_NTT_TABLES_H */
