// vaese128.vv vd, vs2

#include "zvkned_ext_macros.h"
#include "zvk_ext_macros.h"

// Uncomment to enable debug logging of invocations of this instruction.
// #define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for both state and round key element groups.
#define PRIxEG PRIxEGU8x16
#define PRVEG(X) PRVEGU8x16_LE(X)
#else
#define DLOG(...) (void)(0)
#endif

require_vaes_vv_constraints;

// There is one round constant for each round number
// between 1 and 10. We index using 'round# -1'.
static constexpr uint8_t kRoundConstants[10] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

VI_ZVK_VD_VS2_NOOPERANDS_PRELOOP_EGU32x4_NOVM_LOOP(
  {
    DLOG("vaese128_vv vd#(%" PRIuREG ") vs2#(%" PRIuREG ")"
         " vstart_eg(%" PRIuREG ") vl_eg(%" PRIuREG ")",
         vd_num, vs2_num, vstart_eg, vl_eg);
  },
  {},  // No PRELOOP.
  {
    // For AES128, AES192, or AES256, state and key are 128b/16B values:
    //  - vd contains the input state,
    //  - vs2 contains the round key,
    //  - vd receives the output state.
    //
    // While the spec calls for handling the vector as made of EGU32x4
    // element groups (i.e., 4 uint32_t), it is convenient to treat
    // AES state and key as EGU8x16 (i.e., 16 uint8_t). This is why
    // we extract the operands here instead of using the existing LOOP
    // macro that defines/extracts the operand variables as EGU32x4.
    //EGU8x16_t aes_state = P.VU.elt_group<EGU8x16_t>(vd_num, idx_eg);
    //const EGU8x16_t round_key = P.VU.elt_group<EGU8x16_t>(vs2_num, idx_eg);
    EGU8x16_t aes_state = P.VU.elt_group<EGU8x16_t>(vd_num, idx_eg);
    EGU32x4_t round_key = P.VU.elt_group<EGU32x4_t>(vs2_num, idx_eg);
    // EGU32x4_t &round_key = orig_key;

    // EGU8x16_t &aes_state = plaintext;

    DLOG("vaese128: @start"
         " plaintext(%" PRIxEG "), key(%" PRIxEGU32x4_LE ") idx_eg(%" PRIuREG ")",
         PRVEG(aes_state), PRVEGU32x4_LE(round_key), idx_eg);
    EGU8x16_t &vd = P.VU.elt_group<EGU8x16_t>(vd_num, idx_eg, true);

    // first round
    EGU8x16_XOREQ_EGU32x4(aes_state, round_key);
    DLOG("after frst round: @aes_state(%" PRIxEG ")", PRVEG(aes_state));

    int roundId;

    // middle round round 1 .. 10
    for (roundId = 1; roundId < 10; roundId++) {
      // expanding key
      // 'temp' is extracted from the last (most significant) word of key[i].
      uint32_t temp = round_key[3];
      const uint32_t rcon = kRoundConstants[roundId - 1];
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
      DLOG("vaeskf1 old %" PRIxEGU32x4_LE, PRVEGU32x4_LE(round_key));

      const uint32_t w0 = round_key[0] ^ temp;
      const uint32_t w1 = round_key[1] ^ w0;
      const uint32_t w2 = round_key[2] ^ w1;
      const uint32_t w3 = round_key[3] ^ w2;

      // Overwrite vd with k[i+1] from the new words.
      SET_EGU32x4_LE(round_key, w0, w1, w2, w3);

      // actual middle round
      // SubBytes - Apply S-box to every byte in the state
      VAES_SUB_BYTES(aes_state);
      DLOG("vaesem: @s_box state(%" PRIxEG ")", PRVEG(aes_state));
      // ShiftRows - Rotate each row bytes by 0, 1, 2, 3 positions.
      VAES_SHIFT_ROWS(aes_state);
      DLOG("vaesem: @s_row state(%" PRIxEG ")", PRVEG(aes_state));
      // MixColumns
      VAES_MIX_COLUMNS(aes_state);
      DLOG("vaesem: @m_col state(%" PRIxEG ")", PRVEG(aes_state));
      // AddRoundKey
      DLOG("vaesem: @k_sch round_key(%" PRIxEGU32x4_LE ")", PRVEGU32x4_LE(round_key));
      EGU8x16_XOREQ_EGU32x4(aes_state, round_key);
      DLOG("vaesem: @output state(%" PRIxEG ")", PRVEG(aes_state));
    }

    // last key expansion
    // 'temp' is extracted from the last (most significant) word of key[i].
    uint32_t temp = round_key[3];
    DLOG("vaeskf1 temp(%08x) vs2 word3", PRVn32(temp));
    temp = (temp >> 8) | (temp << 24);  // Rotate right by 8
    DLOG("vaeskf1: temp(%08x) after RotWord", PRVn32(temp));
    temp = (((uint32_t)AES_ENC_SBOX[(temp >> 24) & 0xFF] << 24) |
            ((uint32_t)AES_ENC_SBOX[(temp >> 16) & 0xFF] << 16) |
            ((uint32_t)AES_ENC_SBOX[(temp >>  8) & 0xFF] <<  8) |
            ((uint32_t)AES_ENC_SBOX[(temp >>  0) & 0xFF] <<  0));
    DLOG("vaeskf1: temp(%08x) after SubWord", PRVn32(temp));

    const uint32_t rcon = kRoundConstants[roundId - 1];
    temp = temp ^ rcon;
    DLOG("vaeskf1 temp(%08x) after xor w/ rcon", PRVn32(temp));

    // "old" words are the w[i-Nk] of FIPS-197. They are extracted
    // from vs2, which contains key[i] in AES-128 where Nk=4.
    DLOG("vaeskf1 old %" PRIxEGU32x4_LE "", PRVEGU32x4_LE(round_key));

    const uint32_t w0 = round_key[0] ^ temp;
    const uint32_t w1 = round_key[1] ^ w0;
    const uint32_t w2 = round_key[2] ^ w1;
    const uint32_t w3 = round_key[3] ^ w2;

    // Overwrite vd with k[i+1] from the new words.
    SET_EGU32x4_LE(round_key, w0, w1, w2, w3);

    // final round
    // SubBytes - Apply S-box to every byte in the state
    VAES_SUB_BYTES(aes_state);
    DLOG("vaesef: @s_box state(%" PRIxEG ")", PRVEG(aes_state));
    // ShiftRows - Rotate each row bytes by 0, 1, 2, 3 positions.
    VAES_SHIFT_ROWS(aes_state);
    DLOG("vaesef: @s_row state(%" PRIxEG ")", PRVEG(aes_state));
    // MixColumns is not performed for the final round.
    // AddRoundKey
    DLOG("vaesef: @k_sch round_key(%" PRIxEGU32x4_LE ")", PRVEGU32x4_LE(round_key));
    EGU8x16_XOREQ_EGU32x4(aes_state, round_key);
    DLOG("vaesef: @output state(%" PRIxEG ")", PRVEG(aes_state));

    // Update the destination register.
    EGU8x16_COPY(vd, aes_state);
  }
);
