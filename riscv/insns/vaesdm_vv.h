// vaesdm.vv vd, vs2

#include "zvkned_ext_macros.h"
#include "zvk_ext_macros.h"

// Uncomment to enble debug logging of invocations of this instruction.
//#define DLOG_INVOCATION

#if defined(DLOG_INVOCATION)
#define DLOG(...) ZVK_DBG_LOG(__VA_ARGS__)
// Print format/value for both state and round key element groups.
#define PRIxEG PRIxEGU8x16
#define PRVEG(X) PRVEGU8x16_LE(X)
#else
#define DLOG(...) (void)(0)
#endif

require_vaes_vv_constraints;

VI_ZVK_VD_VS2_NOOPERANDS_PRELOOP_EGU32x4_NOVM_LOOP(
  {
    DLOG("vaesdm_vv vd#(%" PRIuREG ") vs2#(%" PRIuREG ")"
         " vstart_eg(%" PRIuREG ") vl_eg(%" PRIuREG ")",
         vd_num, vs2_num, vstart_eg, vl_eg);
  },
  {},  // No PRELOOP.
  {
    // For AES128, AES192, or AES256, state and key are 128b/16B values:
    //  - vd contains the input state,
    //  - vs2 contains the round key,
    //  - vd does receive the output state.
    //
    // While the spec calls for handling the vector as made of EGU32x4
    // element groups (i.e., 4 uint32_t), it is convenient to treat
    // AES state and key as EGU8x16 (i.e., 16 uint8_t). This is why
    // we extract the operands here instead of using the existing LOOP
    // macro that defines/extracts the operand variables as EGU32x4.
    EGU8x16_t aes_state = P.VU.elt_group<EGU8x16_t>(vd_num, idx_eg);
    const EGU8x16_t round_key = P.VU.elt_group<EGU8x16_t>(vs2_num, idx_eg);

    DLOG("vaesdm: @start state(%" PRIxEG "), key(%" PRIxEG ")"
	 " idx_eg(%" PRIuREG ")",
         PRVEG(aes_state), PRVEG(round_key), idx_eg);

    // InvShiftRows - Rotate each row bytes by 0, 1, 2, 3 positions.
    VAES_INV_SHIFT_ROWS(aes_state);
    DLOG("vaesdm: @is_row state(%" PRIxEG ")", PRVEG(aes_state));
    // InvSubBytes - Apply S-box to every byte in the state
    VAES_INV_SUB_BYTES(aes_state);
    DLOG("vaesdm: @is_box state(%" PRIxEG ")", PRVEG(aes_state));
    // AddRoundKey (which is also InvAddRoundKey as it's xor)
    DLOG("vaesdm: ik_sched round_key(%" PRIxEG ")", PRVEG(round_key));
    EGU8x16_XOREQ(aes_state, round_key);
    DLOG("vaesdm: @ik_add cipher state(%" PRIxEG ")", PRVEG(aes_state));
    // InvMixColumns
    VAES_INV_MIX_COLUMNS(aes_state);
    DLOG("vaesdm: @ioutput state(%" PRIxEG ")", PRVEG(aes_state));

    // Update the destination register.
    EGU8x16_t &vd = P.VU.elt_group<EGU8x16_t>(vd_num, idx_eg, true);
    EGU8x16_COPY(vd, aes_state);
  }
);
