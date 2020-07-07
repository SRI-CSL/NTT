/*
 * Parameters:
 * - q = 12289
 * - k = 3
 * - n = 2048
 * - psi = 12248
 * - omega = psi^2 = 1681
 * - inverse of psi = 7793
 * - inverse of omega = 10900
 * - inverse of n = 12283
 * - inverse of k = 8193
 */

#ifndef __NTT_RED2048_TABLES_H
#define __NTT_RED2048_TABLES_H

#include <stdint.h>

/*
 * PARAMETERS
 */
static const int32_t ntt_red2048_psi = 12248;
static const int32_t ntt_red2048_omega = 1681;
static const int32_t ntt_red2048_inv_psi = 7793;
static const int32_t ntt_red2048_inv_omega = 10900;
static const int32_t ntt_red2048_inv_n = 12283;
static const int32_t ntt_red2048_inv_k = 8193;
static const int32_t ntt_red2048_rescale = 4175;

/*
 * POWERS OF PSI
 */
extern const int16_t ntt_red2048_psi_powers[2048];
extern const int16_t ntt_red2048_inv_psi_powers[2048];
extern const int16_t ntt_red2048_scaled_inv_psi_powers[2048];

/*
 * TABLES FOR NTT COMPUTATION
 */
extern const int16_t ntt_red2048_omega_powers[2048];
extern const int16_t ntt_red2048_omega_powers_rev[2048];
extern const int16_t ntt_red2048_inv_omega_powers[2048];
extern const int16_t ntt_red2048_inv_omega_powers_rev[2048];
extern const int16_t ntt_red2048_mixed_powers[2048];
extern const int16_t ntt_red2048_mixed_powers_rev[2048];
extern const int16_t ntt_red2048_inv_mixed_powers[2048];
extern const int16_t ntt_red2048_inv_mixed_powers_rev[2048];

#endif /* __NTT_RED2048_TABLES_H */
