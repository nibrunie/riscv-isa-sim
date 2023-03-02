// vaesz.vs vd, vs2

#include "zvk_ext_macros.h"
#include "zvkned_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for both state and round key element groups.
#define PRIxEG PRIxEGU8x16
#define PRVEG(X) PRVEGU8x16_LE(X)
#else
#define DLOG(...) (void)(0)
#endif

require_vaes_vs_constraints;

VI_ZVK_VD_VS2_NOOPERANDS_PRELOOP_EGU32x4_NOVM_LOOP(
  {
    DLOG("vaesz_vs vd#(%" PRIuREG ") vs2#(%" PRIuREG ")"
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
  const EGU8x16_t scalar_key = P.VU.elt_group<EGU8x16_t>(vs2_num, 0);,
  {
    EGU8x16_t &vd = P.VU.elt_group<EGU8x16_t>(vd_num, idx_eg, true);
    DLOG("vaesz: @start"
         " vd_in(%" PRIxEG ") scalar vs2(%" PRIxEG ") idx_eg(%" PRIuREG ")",
         PRVEG(vd), PRVEG(scalar_key), idx_eg);

    // Produce vd = vd ^ "common key from vs2".
    EGU8x16_XOR(vd, vd, scalar_key);
  }
);
