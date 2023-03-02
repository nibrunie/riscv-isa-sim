// vsm4r.vs vd, vs2

#include "zvksed_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for both state and round key element groups.
#define PRIxEG PRIxEGU32x4_LE
#define PRVEG(X) PRVEGU32x4_LE(X)
// Print format/value for "v<reg_num>(<Element Group in Hex, Big Endian>)"
#define PRI_uR_xEG PRI_uREG_xEGU32x4
#define PRV_R_EG(reg_num, reg) PRV_REG_EGU32x4_LE(reg_num, reg)
#else
#define DLOG(...) (void)(0)
#endif

require_vsm4_constraints;
// No overlap of vd and vs2.
require(insn.rd() != insn.rs2());

VI_ZVK_VD_VS2_NOOPERANDS_PRELOOP_EGU32x4_NOVM_LOOP(
  {
    DLOG("vsm4r_vs vd#(%" PRIuREG ") vs2#(%" PRIuREG ")"
         " vstart_eg(%" PRIuREG ") vl_eg(%" PRIuREG ")",
         vd_num, vs2_num, vstart_eg, vl_eg);
  },
  // This statement will be executed before the first execution
  // of the loop, and only if the loop is going to be entered.
  // We cannot use a block ( { ... } ) since we want the variables declared
  // here to be visible in the loop block.
  // We capture the "scalar", vs2's first element, by copy, even though
  // the "no overlap" constraint means that vs2 should remain constant
  // during the loop.
  const EGU32x4_t scalar_key = P.VU.elt_group<EGU32x4_t>(vs2_num, 0);
  const uint32_t rk0 = scalar_key[0];
  const uint32_t rk1 = scalar_key[1];
  const uint32_t rk2 = scalar_key[2];
  const uint32_t rk3 = scalar_key[3];,
  {
    EGU32x4_t &state = P.VU.elt_group<EGU32x4_t>(vd_num, idx_eg, true);

    DLOG("vsm4r_vs " PRI_uR_xEG " " PRI_uR_xEG,
         PRV_R_EG(vd_num, state), PRV_R_EG(vs2_num, scalar_key));

    // {x0, x1,x2, x3} <- vd
    EXTRACT_EGU32x4_WORDS_LE(state, x0, x1, x2, x3);

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
    SET_EGU32x4_LE(state, x4, x5, x6, x7);
    DLOG("= vsm4r_vs v%" PRIuREG " <- (%" PRIxEG ")", vd_num, PRVEG(state));
  }
);
