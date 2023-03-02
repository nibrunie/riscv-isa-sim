// vsha2cl.vv vd, vs2, vs1

#include "zvknh_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
#else
#define DLOG(...) (void)(0)
#endif

// Ensures VSEW is 32 or 64, and vd doesn't overlap with either vs1 or vs2.
require_vsha2_common_constraints;

switch (P.VU.vsew) {
  case e32: {
#if defined(DLOG_INVOCATION)
// Print format/value for "v<reg_num>(<Element Group in Hex, Big Endian>)"
#define PRI_uR_xEG PRI_uREG_xEGU32x4
#define PRV_R_EG(reg_num, reg) PRV_REG_EGU32x4_BE(reg_num, reg)
#endif

    require_vsha2_vsew32_constraints;

    VI_ZVK_VD_VS1_VS2_EGU32x4_NOVM_LOOP(
      {
         DLOG("-- vsha2cl_vv(256) " ZVK_PRI_REGNUMS_VD_VS2_VS1,
              ZVK_PRV_REGNUMS_VD_VS2_VS1);
      },
      {
        DLOG("vsha2cl " PRI_uR_xEG " " PRI_uR_xEG " " PRI_uR_xEG,
             PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2),
             PRV_R_EG(vs1_num, vs1));

        // {c, d, g, h} <- vd
        EXTRACT_EGU32x4_WORDS_BE(vd, c, d, g, h);
        // {a, b, e, f}  <- vs2
        EXTRACT_EGU32x4_WORDS_BE(vs2, a, b, e, f);
        // {kw3, kw2, kw1, kw0} <- vs1.  "kw" stands for K+W
        EXTRACT_EGU32x4_WORDS_BE(vs1, _unused_kw3, _unused_kw2, kw1, kw0);

        ZVK_SHA256_COMPRESS(a, b, c, d, e, f, g, h, kw0);
        ZVK_SHA256_COMPRESS(a, b, c, d, e, f, g, h, kw1);

        // Update the destination register, vd <- {a, b, e, f}.
        SET_EGU32x4_BE(vd, a, b, e, f);
        DLOG("= vsha2cl " PRI_uR_xEG, PRV_R_EG(vd_num, vd));
      }
    );
    break;
#undef PRI_uR_xEG
#undef PRV_R_EG
  }

  case e64: {
#if defined(DLOG_INVOCATION)
// Print format/value for "v<reg_num>(<Element Group in Hex, Big Endian>)"
#define PRI_uR_xEG PRI_uREG_xEGU64x4
#define PRV_R_EG(reg_num, reg) PRV_REG_EGU64x4_BE(reg_num, reg)
#endif

    require_vsha2_vsew64_constraints;

    VI_ZVK_VD_VS1_VS2_EGU64x4_NOVM_LOOP(
      {
        DLOG("-- vsha2cl_vv(512) " ZVK_PRI_REGNUMS_VD_VS2_VS1,
             ZVK_PRV_REGNUMS_VD_VS2_VS1);
      },
      {
        DLOG("vsha2cl " PRI_uR_xEG " " PRI_uR_xEG " " PRI_uR_xEG,
             PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2),
             PRV_R_EG(vs1_num, vs1));

        // {c, d, g, h} <- vd
        EXTRACT_EGU64x4_WORDS_BE(vd, c, d, g, h);
        // {a, b, e, f}  <- vs2
        EXTRACT_EGU64x4_WORDS_BE(vs2, a, b, e, f);
        // {kw3, kw2, kw1, kw0} <- vs1.  "kw" stands for K+W
        EXTRACT_EGU64x4_WORDS_BE(vs1, _unused_kw3, _unused_kw2, kw1, kw0);

        ZVK_SHA512_COMPRESS(a, b, c, d, e, f, g, h, kw0);
        ZVK_SHA512_COMPRESS(a, b, c, d, e, f, g, h, kw1);

        // Update the destination register, vd <- {a, b, e, f}.
        SET_EGU64x4_BE(vd, a, b, e, f);
        DLOG("= vsha2cl " PRI_uR_xEG, PRV_R_EG(vd_num, vd));
      }
    );
    break;
#undef PRI_uR_xEG
#undef PRV_R_EG
  }

  // 'require_vsha2_common_constraints' ensures that
  // VSEW is either 32 or 64.
  default:
    require(false);
}

