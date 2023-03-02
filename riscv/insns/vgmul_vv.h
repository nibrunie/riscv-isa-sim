// vgmul.vv vd, vs2

#include "zvk_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for "v<reg_num>(<Element Group in Hex, Big Endian>)"
#define PRI_uR_xEG PRI_uREG_xEGU32x4
#define PRV_R_EG(reg_num, reg) PRV_REG_EGU32x4_LE(reg_num, reg)
#else
#define DLOG(...) (void)(0)
#endif

require_zvkg;
require_vsew(32);
require_egw_fits(128);

VI_ZVK_VD_VS2_EGU32x4_NOVM_LOOP(
  {},
  {
    DLOG("vgmul Y=" PRI_uR_xEG " H=" PRI_uR_xEG,
         PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2));

    EGU32x4_t Y = vd;  // Multiplier
    EGU32x4_BREV8(Y);
    EGU32x4_t H = vs2;  // Multiplicand
    EGU32x4_BREV8(H);
    EGU32x4_t Z = {};

    for (int bit = 0; bit < 128; bit++) {
      if (EGU32x4_ISSET(Y, bit)) {
        EGU32x4_XOREQ(Z, H);
      }

      bool reduce = EGU32x4_ISSET(H, 127);
      EGU32x4_LSHIFT(H);  // Lef shift by 1
      if (reduce) {
        H[0] ^= 0x87; // Reduce using x^7 + x^2 + x^1 + 1 polynomial
      }
    }
    EGU32x4_BREV8(Z);
    vd = Z;
  }
);
