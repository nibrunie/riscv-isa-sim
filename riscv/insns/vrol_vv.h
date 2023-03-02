// vrol.vv vd, vs2, vs1, vm

#include "zvk_ext_macros.h"

require_zvkb;

// 'mask' selects the low log2(vsew) bits of the shift amount,
// to limit the maximum shift to "vsew - 1" bits.
const reg_t mask = P.VU.vsew - 1;

VI_VV_ULOOP
({
  // For .vv, the shift amount comes from the vs1 element.
  const reg_t shift = vs1 & mask;
  if (shift != 0) {  // Protect against UB.
    vd = (vs2 << shift) | (vs2 >> (sew - shift));
  } else {
    vd = vs2;
  }
})
