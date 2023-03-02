// vror.vi vd, vs2, zimm6, vm

#include "zvk_ext_macros.h"

require_zvkb;

// 'mask' selects the low log2(vsew) bits of the shift amount,
// to limit the maximum shift to "vsew - 1" bits.
const reg_t mask = P.VU.vsew - 1;

// For .vi, the shift amount comes from bits [26,19-15].
const reg_t shift = insn.v_zimm6() & mask;

VI_VX_ULOOP
({
  if (shift != 0) {  // Protect against UB.
    vd = (vs2 >> shift) | (vs2 << (sew - shift));
  } else {
    vd = vs2;
  }
})
