// vrol.vx vd, vs2, rs1, vm

#include "zvk_ext_macros.h"

require_zvkb;

// 'mask' selects the low log2(vsew) bits of the shift amount,
// to limit the maximum shift to "vsew - 1" bits.
const reg_t mask = P.VU.vsew - 1;

// For .vx, the shift amount comes from rs1.
const reg_t shift = ((reg_t)RS1) & mask;

VI_VX_ULOOP
({
  if (shift != 0) {  // Protect against UB.
    vd = (vs2 << shift) | (vs2 >> (sew - shift));
  } else {
    vd = vs2;
  }
})
