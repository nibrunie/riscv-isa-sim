// vsm3c.vi vd, vs2, rnd

#include "zvksh_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for "v<reg_num>(<Element Group in Hex, Little Endian>)"
#define PRI_uR_xEG PRI_uREG_xEGU32x8
#define PRV_R_EG(reg_num, reg) PRV_REG_EGU32x8_LE(reg_num, reg)
#else
#define DLOG(...) (void)(0)
#endif

require_vsm3_constraints;

// Rotates left a uint32_t value by N bits.
//   uint32_t SM3_ROL32(uint32_t X, unsigned int N);
// This is a "safer" version of zvk_ext_macros.h's ROL32 that accepts
// a run-time shift-value between 0 and 31. ROL32 has Undefine Behavior
// when invoked with value 0.
#define SM3_ROL32(X, N) \
    ((N) == 0 ? (X) : (((X) << (N)) | ((X) >> (32 - (N)))))

VI_ZVK_VD_VS2_ZIMM5_EGU32x8_NOVM_LOOP(
  {
    DLOG("-- vsm3c_vi " ZVK_PRI_REGNUMS_VD_VS2_ZIMM5,
         ZVK_PRV_REGNUMS_VD_VS2_ZIMM5);
  },
  // No need to validate or normalize 'zimm5' here as this is a 5 bits value
  // and all values in 0-31 are valid.
  const reg_t round = zimm5;,
  {
    DLOG("vsm3c " PRI_uR_xEG " " PRI_uR_xEG,
         PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2));

    // {H, G, F, E, D, C, B, A} <- vd
    EXTRACT_EGU32x8_WORDS_BE_BSWAP(vd, H, G, F, E, D, C, B, A);
    // {_, _, w5, w4, _, _, w1, w0} <- vs2
    EXTRACT_EGU32x8_WORDS_BE_BSWAP(vs2,
                                   _unused_w7, _unused_w6, w5, w4,
                                   _unused_w3, _unused_w2, w1, w0);
    const uint32_t x0 = w0 ^ w4;  // W'[0] in spec documentation.
    const uint32_t x1 = w1 ^ w5;  // W'[1]

    // Two rounds of compression.
    uint32_t ss1;
    uint32_t ss2;
    uint32_t tt1;
    uint32_t tt2;
    uint32_t j;

    j = 2 * round;
    ss1 = SM3_ROL32(SM3_ROL32(A, 12) + E + SM3_ROL32(ZVKSH_T(j), j % 32), 7);
    ss2 = ss1 ^ SM3_ROL32(A, 12);
    tt1 = ZVKSH_FF(A, B, C, j) + D + ss2 + x0;
    tt2 = ZVKSH_GG(E, F, G, j) + H + ss1 + w0;
    D = C;
    const uint32_t C1 = SM3_ROL32(B, 9);
    B = A;
    const uint32_t A1 = tt1;
    H = G;
    const uint32_t G1 = SM3_ROL32(F, 19);
    F = E;
    const uint32_t E1 = ZVKSH_P0(tt2);

    j = 2 * round + 1;
    ss1 = SM3_ROL32(SM3_ROL32(A1, 12) + E1 + SM3_ROL32(ZVKSH_T(j), j % 32), 7);
    ss2 = ss1 ^ SM3_ROL32(A1, 12);
    tt1 = ZVKSH_FF(A1, B, C1, j) + D + ss2 + x1;
    tt2 = ZVKSH_GG(E1, F, G1, j) + H + ss1 + w1;
    D = C1;
    const uint32_t C2 = SM3_ROL32(B, 9);
    B = A1;
    const uint32_t A2 = tt1;
    H = G1;
    const uint32_t G2 = SM3_ROL32(F, 19);
    F = E1;
    const uint32_t E2 = ZVKSH_P0(tt2);

    // Update the destination register.
    SET_EGU32x8_WORDS_BE_BSWAP(vd, G1, G2, E1, E2, C1, C2, A1, A2);
    DLOG("= vsm3c " PRI_uR_xEG, PRV_R_EG(vd_num, vd));
  }
);

#undef SM3_ROL32
