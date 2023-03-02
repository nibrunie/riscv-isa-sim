// vshams.vv vd, vs2, vs1

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
         DLOG("-- vsha2ms_vv(256) " ZVK_PRI_REGNUMS_VD_VS2_VS1,
              ZVK_PRV_REGNUMS_VD_VS2_VS1);
      },
      {
        DLOG("vsha2ms " PRI_uR_xEG " " PRI_uR_xEG " " PRI_uR_xEG,
             PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2),
             PRV_R_EG(vs1_num, vs1));

        // {w3, w2, w1, w0} <- vd
        EXTRACT_EGU32x4_WORDS_BE(vd, w3, w2, w1, w0);
        // {w11, w10, w9, w4} <- vs2
        EXTRACT_EGU32x4_WORDS_BE(vs2, w11, w10, w9, w4);
        // {w15, w14, w13, w12} <- vs1
        EXTRACT_EGU32x4_WORDS_BE(vs1, w15, w14, w13, w12);

        const uint32_t w16 = ZVK_SHA256_SCHEDULE(w14,  w9, w1, w0);
        const uint32_t w17 = ZVK_SHA256_SCHEDULE(w15, w10, w2, w1);
        const uint32_t w18 = ZVK_SHA256_SCHEDULE(w16, w11, w3, w2);
        const uint32_t w19 = ZVK_SHA256_SCHEDULE(w17, w12, w4, w3);

        // Update the destination register.
        SET_EGU32x4_BE(vd, w19, w18, w17, w16);;
        DLOG("= vsha2ms " PRI_uR_xEG, PRV_R_EG(vd_num, vd));
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
        DLOG("-- vsha2ms_vv(512) " ZVK_PRI_REGNUMS_VD_VS2_VS1,
             ZVK_PRV_REGNUMS_VD_VS2_VS1);
      },
      {
        DLOG("vsha2ms " PRI_uR_xEG " " PRI_uR_xEG " " PRI_uR_xEG,
             PRV_R_EG(vd_num, vd), PRV_R_EG(vs2_num, vs2),
             PRV_R_EG(vs1_num, vs1));

        // {w3, w2, w1, w0} <- vd
        EXTRACT_EGU64x4_WORDS_BE(vd, w3, w2, w1, w0);
        // {w11, w10, w9, w4} <- vs2
        EXTRACT_EGU64x4_WORDS_BE(vs2, w11, w10, w9, w4);
        // {w15, w14, w13, w12} <- vs1
        EXTRACT_EGU64x4_WORDS_BE(vs1, w15, w14, w13, w12);

        const uint64_t w16 = ZVK_SHA512_SCHEDULE(w14,  w9, w1, w0);
        const uint64_t w17 = ZVK_SHA512_SCHEDULE(w15, w10, w2, w1);
        const uint64_t w18 = ZVK_SHA512_SCHEDULE(w16, w11, w3, w2);
        const uint64_t w19 = ZVK_SHA512_SCHEDULE(w17, w12, w4, w3);

        // Update the destination register.
        SET_EGU64x4_BE(vd, w19, w18, w17, w16);;
        DLOG("= vsha2ms " PRI_uR_xEG, PRV_R_EG(vd_num, vd));
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
