// vrev8.v vd, vs2, vm

#include "zvk_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
#else
#define DLOG(...) (void)(0)
#endif

require_zvkb;

VI_VX_ULOOP  // Note that rs1 is unused.
({
  vd = vs2;
  if (P.VU.vsew > 8)
    vd = ((vd & 0x00FF00FF00FF00FFllu) <<  8) | ((vd & 0xFF00FF00FF00FF00llu) >>  8);
  if (P.VU.vsew > 16)
    vd = ((vd & 0x0000FFFF0000FFFFllu) << 16) | ((vd & 0xFFFF0000FFFF0000llu) >> 16);
  if (P.VU.vsew > 32)
    vd = ((vd & 0x00000000FFFFFFFFllu) << 32) | ((vd & 0xFFFFFFFF00000000llu) >> 32);
})
