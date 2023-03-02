// vaeskf1.vi vd, vs2, rnd

#include "zvk_ext_macros.h"
#include "zvkned_ext_macros.h"

// Uncomment to enble debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for both state and round key element groups.
#define PRIxEG PRIxEGU32x4_LELE
#define PRVEG(X) PRVEGU32x4_LELE(X)
#else
#define DLOG(...) (void)(0)
#endif

require_vaeskf_vi_constraints;

// There is one round constant for each round number
// between 1 and 10. We index using 'round# -1'.
static constexpr uint8_t kRoundConstants[10] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

// For AES128, AES192, or AES256, keys (and state) are handled as
// 128b/16B values.
//
// The Zvkned spec calls for handling the vector as made of EGU32x4
// element groups (i.e., 4 uint32_t), and FIPS-197 AES specification
// describes the key expansion in terms of manipulations of 32 bit
// words, so using the EGU32x4 is natural.
//
VI_ZVK_VD_VS2_ZIMM5_EGU32x4_NOVM_LOOP(
  {
    DLOG("vaeskf1_vi " ZVK_PRI_REGNUMS_VD_VS2_ZIMM5,
         ZVK_PRV_REGNUMS_VD_VS2_ZIMM5);
  },
  // The following statements will be executed before the first execution
  // of the loop, and only if the loop is going to be entered.
  // We cannot use a block ( { ... } ) since we want the 'round' variable
  // declared and defined here  here to be visible in the loop block.
  // Only consider the bottom 4 bits of the immediate.
  const reg_t zimm4 = zimm5 & 0xF;
  // Normalize the round value to be in [2, 14] by toggling bit 3
  // if outside the range (i.e., +8 or -8).
  const reg_t round = ((1 <= zimm4) && (zimm4 <= 10)) ? zimm4 : (zimm4 ^ 0x8);
  if (round != zimm5) {
    DLOG("vaeskf2: zimm5 %" PRIuREG " => round %" PRIuREG, zimm5, round);
  }
  const uint32_t rcon = kRoundConstants[round - 1];
  DLOG("vaeskf1 rcon(%08x)", PRVn32(rcon));,
  {
    // vaeskf1_vi produces key[i+1] in vd, it receives key[i] in vs2,
    // i.e., 4x32b values (4 words).
    //
    // The logic is fairly similar between vaeskf1/vaeskf2, with the following
    // differences:
    // - in AES-128 (vaeskf1), we get both the 'temp' word and
    //   the "previous words" w0..w3 from key[i]/vs2.
    // - in AES-256 (vaeskf2), we get 'temp' from key[i]/vs2, and
    //   the "previous words" w0..w3 from key[i-1]/vd.
    DLOG("vaeskf1 processing one group vs2(%" PRIxEG ") idx_eg(%" PRIuREG ")",
         PRVEG(vs2), idx_eg);

    // 'temp' is extracted from the last (most significant) word of key[i].
    uint32_t temp = vs2[3];
    DLOG("vaeskf1 temp(%08x) vs2 word3", PRVn32(temp));
    temp = (temp >> 8) | (temp << 24);  // Rotate right by 8
    DLOG("vaeskf1: temp(%08x) after RotWord", PRVn32(temp));
    temp = (((uint32_t)AES_ENC_SBOX[(temp >> 24) & 0xFF] << 24) |
            ((uint32_t)AES_ENC_SBOX[(temp >> 16) & 0xFF] << 16) |
            ((uint32_t)AES_ENC_SBOX[(temp >>  8) & 0xFF] <<  8) |
            ((uint32_t)AES_ENC_SBOX[(temp >>  0) & 0xFF] <<  0));
    DLOG("vaeskf1: temp(%08x) after SubWord", PRVn32(temp));

    temp = temp ^ rcon;
    DLOG("vaeskf1 temp(%08x) after xor w/ rcon", PRVn32(temp));

    // "old" words are the w[i-Nk] of FIPS-197. They are extracted
    // from vs2, which contains key[i] in AES-128 where Nk=4.
    DLOG("vaeskf1 old %" PRIxEG, PRVEG(vs2));

    const uint32_t w0 = vs2[0] ^ temp;
    const uint32_t w1 = vs2[1] ^ w0;
    const uint32_t w2 = vs2[2] ^ w1;
    const uint32_t w3 = vs2[3] ^ w2;

    // Overwrite vd with k[i+1] from the new words.
    SET_EGU32x4_LE(vd, w0, w1, w2, w3);
    DLOG("= vaeskf1 new %" PRIxEG, PRVEG(vd));
  }
);
