// vsm4k.vi vd, vs2, round#

#include "zvksed_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for both state and round key element groups.
#define PRIxEG PRIxEGU32x4_LE
#define PRVEG(X) PRVEGU32x4_LE(X)
// Print format/value for "v<reg_num>(<Element Group in Hex, Little Endian>)"
#define PRI_uR_xEG PRI_uREG_xEGU32x8
#define PRV_R_EG(reg_num, reg) PRV_REG_EGU32x8_LE(reg_num, reg)
#else
#define DLOG(...) (void)(0)
#endif

require_vsm4_constraints;

VI_ZVK_VD_VS2_ZIMM5_EGU32x4_NOVM_LOOP(
  {
    DLOG("-- vsm4k_vi " ZVK_PRI_REGNUMS_VD_VS2_ZIMM5,
         ZVK_PRV_REGNUMS_VD_VS2_ZIMM5);
  },
  // The following statements will be executed before the first execution
  // of the loop, and only if the loop is going to be entered.
  // We cannot use a block ( { ... } ) since we want the 'round' variable
  // declared and defined here  here to be visible in the loop block.
  // Only consider the bottom 3 bits of the immediate, ensuring that
  // 'round' is in the valid range [0, 7].
  const reg_t round = zimm5 & 0x7;
  if (round != zimm5) {
    DLOG("vsm4k: zimm5 %" PRIuREG " => round %" PRIuREG, zimm5, round);
  },
  {
    // {rk0, rk1, rk2, rk3} <- vs2
    EXTRACT_EGU32x4_WORDS_LE(vs2, rk0, rk1, rk2, rk3);

    uint32_t B = rk1 ^ rk2 ^ rk3 ^ ZVKSED_CK(4 * round);
    uint32_t S = ZVKSED_SUB_BYTES(B);
    uint32_t rk4 = ZVKSED_ROUND_KEY(rk0, S);

    B = rk2 ^ rk3 ^ rk4 ^ ZVKSED_CK(4 * round + 1);
    S = ZVKSED_SUB_BYTES(B);
    uint32_t rk5 = ZVKSED_ROUND_KEY(rk1, S);

    B = rk3 ^ rk4 ^ rk5 ^ ZVKSED_CK(4 * round + 2);
    S = ZVKSED_SUB_BYTES(B);
    uint32_t rk6 = ZVKSED_ROUND_KEY(rk2, S);

    B = rk4 ^ rk5 ^ rk6 ^ ZVKSED_CK(4 * round + 3);
    S = ZVKSED_SUB_BYTES(B);
    uint32_t rk7 = ZVKSED_ROUND_KEY(rk3, S);

    // Update the destination register.
    SET_EGU32x4_LE(vd, rk4, rk5, rk6, rk7);
    DLOG("= vsm4k_vi %" PRIxEG, PRVEG(vd));
  }
);
