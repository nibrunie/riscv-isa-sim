// vclmul.vv vd, vs2, vs1, vm

#include "zvk_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
#else
#define DLOG(...) (void)(0)
#endif

require_zvkb;
require_vsew(64);

VI_VV_ULOOP
({
  // Perform a carryless multiplication 64bx64b on each 64b element,
  // return the low 64b of the 128b product.
  //   <https://en.wikipedia.org/wiki/Carry-less_product>
  vd = 0;
  for (std::size_t bit_idx = 0; bit_idx < sew; ++bit_idx) {
    const reg_t mask = ((reg_t) 1) << bit_idx;
    if ((vs1 & mask) != 0) {
      vd ^= vs2 << bit_idx;
    }
  }
  DLOG("vclmul_vv vd(%" PRIx64 ") <- vs2(%" PRIx64 ") x vs1(%" PRIx64 ")",
       (uint64_t)vd, (uint64_t)vs2, (uint64_t)vs1);
})
