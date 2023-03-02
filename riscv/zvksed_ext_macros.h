// Helper macros and functions to help implement instructions defined as part of
// the RISC-V Zvksed extension (vectorized SM4).

#include "insns/sm4_common.h"
#include "zvk_ext_macros.h"

#ifndef RISCV_INSNS_ZVKSED_COMMON_H_
#define RISCV_INSNS_ZVKSED_COMMON_H_

// Constraints common to all vsm4* instructions:
//  - Zvksed is enabled
//  - VSEW == 32
//  - EGW (128) <= LMUL * VLEN
//
// The constraint that vstart and vl are both EGS (4) aligned
// is checked in the VI_ZVK_..._EGU32x4_..._LOOP macros.
#define require_vsm4_constraints \
  do { \
    require_zvksed; \
    require_vsew(32); \
    require_egw_fits(128); \
  } while (false)

// Get byte BYTE of the Constant Key.
#define ZVKSED_CK(BYTE) (ck[(BYTE)])

// Get byte BYTE of the SBox.
#define ZVKSED_SBOX(BYTE)  (sm4_sbox[(BYTE)])

// Apply the nonlinear transformation tau to a 32 bit word B - section 6.2.1.
// of the IETF draft.
#define ZVKSED_SUB_BYTES(B) \
  U32_FROM_U8_LE(ZVKSED_SBOX(EXTRACT_U8((B), 0)), \
                 ZVKSED_SBOX(EXTRACT_U8((B), 1)), \
                 ZVKSED_SBOX(EXTRACT_U8((B), 2)), \
                 ZVKSED_SBOX(EXTRACT_U8((B), 3)))

// Perform the linear transformation L to a 32 bit word S and xor it with a 32
// bit word X - section 6.2.2. of the IETF draft.
#define ZVKSED_ROUND(X, S) \
  ((X) ^ ((S) ^ ROL32((S), 2) ^ ROL32((S), 10) ^ ROL32((S), 18) ^ ROL32((S), 24)))

// Perform the linear transformation L' to a 32 bit word S and xor it with a 32
// bit word X - section 6.2.2. of the IETF draft.
#define ZVKSED_ROUND_KEY(X, S) \
  ((X) ^ ((S) ^ ROL32((S), 13) ^ ROL32((S), 23)))

#endif // RISCV_INSNS_ZVKSED_COMMON_H_
