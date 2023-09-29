// vrol.vx vd, vs2, rs1, vm

#include "zvk_ext_macros.h"

require_zvkb;

// 'mask' selects the low log2(vsew) bits of the shift amount,
// to limit the maximum shift to "vsew - 1" bits.
const reg_t mask = P.VU.vsew - 1;

// For .vx, the shift amount comes from rs1.
const reg_t lshift = ((reg_t)RS1) & mask;
const reg_t rshift = (-lshift) & mask;

VI_V_ULOOP
({
  vd = (vs2 << lshift) | (vs2 >> rshift);
})
