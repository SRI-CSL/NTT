/*
 * Parameters:
 * - q = 12289
 * - n = 32
 * - psi = 563
 * - omega = psi^2 = 9744
 * - inverse of psi = 5828
 * - inverse of omega = 11077
 * - inverse of n = 11905
 */

#ifndef __NTT32_TABLES_H
#define __NTT32_TABLES_H

#include <stdint.h>

/*
 * PARAMETERS
 */
static const int32_t ntt32_psi = 563;
static const int32_t ntt32_omega = 9744;
static const int32_t ntt32_inv_psi = 5828;
static const int32_t ntt32_inv_omega = 11077;
static const int32_t ntt32_inv_n = 11905;

/*
 * BIT-REVERSE SHUFFLE
 */
#define BITREV32_NPAIRS 12

extern const uint16_t ntt32_bitrev[BITREV32_NPAIRS][2];

/*
 * POWERS OF PSI
 */
extern const uint16_t ntt32_psi_powers[32];
extern const uint16_t ntt32_inv_psi_powers[32];
extern const uint16_t ntt32_scaled_inv_psi_powers[32];

/*
 * TABLES FOR NTT COMPUTATION
 */
extern const uint16_t ntt32_omega_powers[32];
extern const uint16_t ntt32_omega_powers_rev[32];
extern const uint16_t ntt32_inv_omega_powers[32];
extern const uint16_t ntt32_inv_omega_powers_rev[32];
extern const uint16_t ntt32_mixed_powers[32];
extern const uint16_t ntt32_mixed_powers_rev[32];
extern const uint16_t ntt32_inv_mixed_powers[32];
extern const uint16_t ntt32_inv_mixed_powers_rev[32];

#endif /* __NTT32_TABLES_H */
