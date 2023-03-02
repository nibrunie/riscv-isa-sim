// vsm4r.vv vd, vs2

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

VI_ZVK_VD_VS2_EGU32x4_NOVM_LOOP(
  {
    DLOG("vsm4r_vv vd#(%" PRIuREG ") vs2#(%" PRIuREG ")"
         " vstart_eg(%" PRIuREG ") vl_eg(%" PRIuREG ")",
         vd_num, vs2_num, vstart_eg, vl_eg);
  },
  {
    DLOG("vsm4r_vv " PRI_uR_xEG " " PRI_uR_xEG,
         PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2));

    // vd = {x0, x1,x2, x3} <- vd
    EXTRACT_EGU32x4_WORDS_LE(vd, x0, x1, x2, x3);
    // {rk0, rk1, rk2, rk3} <- vs2
    EXTRACT_EGU32x4_WORDS_LE(vs2, rk0, rk1, rk2, rk3);

    uint32_t B;
    uint32_t S;

    B = x1 ^ x2 ^ x3 ^ rk0;
    S = ZVKSED_SUB_BYTES(B);
    const uint32_t x4 = ZVKSED_ROUND(x0, S);

    B = x2 ^ x3 ^ x4 ^ rk1;
    S = ZVKSED_SUB_BYTES(B);
    const uint32_t x5 = ZVKSED_ROUND(x1, S);

    B = x3 ^ x4 ^ x5 ^ rk2;
    S = ZVKSED_SUB_BYTES(B);
    const uint32_t x6 = ZVKSED_ROUND(x2, S);

    B = x4 ^ x5 ^ x6 ^ rk3;
    S = ZVKSED_SUB_BYTES(B);
    const uint32_t x7 = ZVKSED_ROUND(x3, S);

    // Update the destination register.
    SET_EGU32x4_LE(vd, x4, x5, x6, x7);
    DLOG("= vsm4r_vv v%" PRIuREG " <- (%" PRIxEG ")", vd_num, PRVEG(vd));
  }
);
