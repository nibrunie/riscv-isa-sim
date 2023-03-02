// Helper macros to help implement instructions defined as part of
// the RISC-V Zvk extension (vector cryptography).

// Note that a good deal of code here would be cleaner/simpler
// if exposed as C++ functions (including templated ones), however
// this is not possible in the contexts where those headers are
// included.

#ifndef RISCV_ZVK_EXT_MACROS_H_
#define RISCV_ZVK_EXT_MACROS_H_

//
// Logging Helpers
//

// Helper format macro to print a reg_t value (which is a uint64_t).
// Using %lu or %llu is not portable across platforms (Linux, macOS).
#define PRIuREG PRIu64
#define PRIxREG PRIx64

// Byte-swap the bytes of a uin32_t such that the order of bytes
// is reversed.
#define BSWAP32(x) \
  ((((uint32_t)((x) >> 24)) & 0xFF) <<  0 | \
   (((uint32_t)((x) >> 16)) & 0xFF) <<  8 | \
   (((uint32_t)((x) >>  8)) & 0xFF) << 16 | \
   (((uint32_t)((x) >>  0)) & 0xFF) << 24)

// Byte-swap the bytes of a uint32_t. This is usually used to display
// a uint32_t encoded in the native little-endian in an order consistent
// with what NIST used in FIPS-197.
#define NIST32(x) BSWAP32(x)

// Helper macro to define DLOGBG_PRINT macros in instruction implementations,
// without having to repeat the same code each time.
#define ZVK_DBG_LOG(FMT, ...) \
  do { \
    printf(FMT "\n", __VA_ARGS__); \
    fflush(stdout); \
  } while (0)

// Helper macros to print uin32_t values in "NIST order".
//   printf("This is the NIST representation: %" PRIn32 "\n",
//          PRVn32(x));
#define PRIn32 "08x"
#define PRVn32(x) NIST32(x)

// Format macro to print a EGU8x16_t (16 bytes) in fixed-width hexadecimal
// form, with separator '_' between every 4-bytes group.
//
// Example:
//   printf(">%" PRIxEGU8x16 "<\n", PRVEGU8x16_LE(my_var));
//   >00010203_04050607_08090a0b_0c0d0e0f<
//
#define PRIxEGU8x16 \
  "02x%02x%02x%02x_%02x%02x%02x%02x_%02x%02x%02x%02x_%02x%02x%02x%02x"
// "Print value" macro to deconstruct a EGU8x16_t variable to match
// a use of PRIxEGU8x16. The bytes are listed in Little Endian (LE) order,
// i.e., with word (byte) at index
#define PRVEGU8x16_LE(X) \
  (X)[ 0], (X)[ 1], (X)[ 2], (X)[ 3], \
  (X)[ 4], (X)[ 5], (X)[ 6], (X)[ 7], \
  (X)[ 8], (X)[ 9], (X)[10], (X)[11], \
  (X)[12], (X)[13], (X)[14], (X)[15]

// Format macro to print a EGU32x4_t (4*4 bytes) in fixed-width hexadecimal
// form, with separator '_' between every 4-bytes word.
// The 4-bytes words are listed in Little Endian (LE) order, i.e., from
// least significant word to most significant one.
//
// Note the similar macros ending in _LELE below, which mimic the NIST
// FIPS-197 order.
//
// Example:
//   printf(">%" PRIxEGU32x4_LE "<\n", PRVEGU32x4_LE(my_var));
//   >03020100_070505040_0b0a0908_0f0e0d0c<
//
#define PRIxEGU32x4_LE "08x_%08x_%08x_%08x"
// "Print value" macro to deconstruct a EGU32x4_t variable to match
// a use of PRIxEGU32x4_LE. The words are listed in Little Endian (LE) order.
#define PRVEGU32x4_LE(X) \
  ((X)[0]), ((X)[1]), ((X)[2]), ((X)[3])

// Format macro to print a EGU32x4_t (4*4 bytes) in fixed-width hexadecimal
// form, with separator '_' between every 4-bytes group.
// This format follows the format used in the AES standard document,
// FIPS-197, where each word *and* each byte in those words is listed
// in Little Endian (LE) order, from least significant to most significant.
// This explains the "LELE" suffix, for LE word order, LE byte order
// in each word.
//
// Example:
//   printf(">%" PRIxEGU32x4_LELE "<\n", PRVEGU32x4_LELE(my_var));
//   >00010203_04050607_08090a0b_0c0d0e0f<
//
#define PRIxEGU32x4_LELE "08x_%08x_%08x_%08x"
// "Print value" macro to deconstruct a EGU32x4_t variable to match
// a use of PRIxEGU32x4_LELE. The bytes are listed in Little Endian (LE) order.
#define PRVEGU32x4_LELE(X) \
  NIST32((X)[0]), NIST32((X)[1]), \
  NIST32((X)[2]), NIST32((X)[3])

// Helper macros to print EGU64x4_t values as 4 uint64_t words,
// listed in big endian (BE) order (most significant word first),
// each word being printed in the "natural" order (from top nibble
// to least nibble), the uint64_t values being printed with the "%016x"
// format specifier.
//
//  printf("This is a 64x4 element group as 4 words: " PRIxEGU64x4 "\n",
//         PRVEGU64x4_BE(x));
#define PRIxEGU64x4 "%016llx__%016llx__%016llx__%016llx"
#define PRVEGU64x4_BE(x) \
    ((x)[3]), ((x)[2]), ((x)[1]), ((x)[0])

// Helper macros to print a EGU64x4_t element group, together with
// the vector number from which is was extracted.
// The vector number is formatted with "%u", the EGU64x4_t value
// with the  PRIxEGU64x4 documented above.
#define PRI_uREG_xEGU64x4 "v%" PRIuREG "(" PRIxEGU64x4 ")"
#define PRV_REG_EGU64x4_BE(reg_num, reg) (reg_num), PRVEGU64x4_BE(reg)

// Debug format/value macros for instructions taking operands
// VD, VS2, VS1. It prints the register numbers of those operands,
// as well as vstart_eg/vl_eg.
//
// This macro is meant to be used with instructions following the template
// "op vd, vs2, vs1". It can be used with instructions using the order
// "op vd, vs1, vs2", however the resulting output can be confusing when
// debugging.
#define ZVK_PRI_REGNUMS_VD_VS2_VS1 \
  "vd#(%" PRIuREG ") vs2#(%" PRIuREG ") vs1#(%" PRIuREG ")" \
  " vstart_eg(%" PRIuREG ") vl_eg(%" PRIuREG ")"
#define ZVK_PRV_REGNUMS_VD_VS2_VS1 vd_num, vs2_num, vs1_num, vstart_eg, vl_eg

// Debug format/value macros for instructions taking operands
// VD, VS2, zimm5. It prints the register numbers of those operands,
// as well as vstart_eg/vl_eg.
//
// This macro is meant to be used with instructions following the template
// "op vd, vs2, round".
#define ZVK_PRI_REGNUMS_VD_VS2_ZIMM5 \
  "vd#(%" PRIuREG ") vs2#(%" PRIuREG ") zimm5(%" PRIuREG ")" \
  " vstart_eg(%" PRIuREG ") vl_eg(%" PRIuREG ")"
#define ZVK_PRV_REGNUMS_VD_VS2_ZIMM5 vd_num, vs2_num, zimm5, vstart_eg, vl_eg

// Helper macros to print EGU32x4_t values as 4 uint32_t words,
// listed in big endian (BE) order (most significant word first),
// each word being printed in the "natural" order (from top nibble
// to least nibble), the uint32_t values being printed with the "%08x"
// format specifier.
//
//  printf("This is a 32x4 element group as 4 words: " PRIxEGU32x4 "\n",
//         PRVEGU32x4_BE(x));
#define PRIxEGU32x4 "%08x__%08x__%08x__%08x"
#define PRVEGU32x4_BE(x) \
    ((x)[3]), ((x)[2]), ((x)[1]), ((x)[0])

// Helper macros to print EGU32x8_t values as 8 uint32_t words,
// listed in little endian (LE) order (most significant word first),
// each word being printed in the "natural" order (from top nibble
// to least nibble), the uint32_t values being printed with the "%08x"
// format specifier.
//
//  printf("This is a 32x8 element group as 8 words: " PRIxEGU32x8 "\n",
//         PRVEGU32x4_LE(x));
#define PRIxEGU32x8 "%08x__%08x__%08x__%08x__%08x__%08x__%08x__%08x_"
#define PRVEGU32x8_LE(x) \
    ((x)[0]), ((x)[1]), ((x)[2]), ((x)[3]), \
    ((x)[4]), ((x)[5]), ((x)[6]), ((x)[7])

// Helper macros to print a EGU32x4_t element group, together with
// the vector number from which is was extracted.
// The vector number is formatted with "%u", the EGU32x4_t value
// with the  PRIxEGU32x4 documented above.
#define PRI_uREG_xEGU32x4 "v%" PRIuREG "(" PRIxEGU32x4 ")"
#define PRV_REG_EGU32x4_BE(reg_num, reg) (reg_num), PRVEGU32x4_BE(reg)
#define PRV_REG_EGU32x4_LE(reg_num, reg) (reg_num), PRVEGU32x4_LE(reg)

// Helper macros to print a EGU32x8_t element group, together with
// the vector number from which is was extracted.
// The vector number is formatted with "%u", the EGU32x8_t value
// with the PRIxEGU32x8 documented above.
#define PRI_uREG_xEGU32x8 "v%" PRIuREG "(" PRIxEGU32x8 ")"
#define PRV_REG_EGU32x8_LE(reg_num, reg) (reg_num), PRVEGU32x8_LE(reg)

//
// Predicate Macros
//

// Ensures that the ZVKB extension (vector crypto bitmanip) is present,
// which depends on vector.
#define require_zvkb \
  do { \
    require_vector(true); \
    require_extension(EXT_ZVKB); \
  } while (0)

// Ensures that the ZVKG extension (vector Gallois Field Multiplication)
// is present, which depends on vector.
#define require_zvkg \
  do { \
    require_vector(true); \
    require_extension(EXT_ZVKG); \
  } while (0)

// Ensures that a ZVK extension supporting SHA-256 is present.
// For SHA-256, this support is present in either Zvknha or Zvknhb.
// Also ensures that vector support is present.
#define require_zvknh_256 \
  do { \
    require_vector(true); \
    require_either_extension(EXT_ZVKNHA, EXT_ZVKNHB); \
  } while (0)

// Ensures that the ZVKNED extension (vector AES single round) is present,
// which depends on vector.
#define require_zvkned \
  do { \
    require_vector(true); \
    require_extension(EXT_ZVKNED); \
  } while (0)

// Ensures that a ZVK extension supporting SHA-512 is present.
// For SHA-512, this support is only present in Zvknhb.
// Also ensures that vector support is present.
#define require_zvknh_512 \
  do { \
    require_vector(true); \
    require_extension(EXT_ZVKNHB); \
  } while (0)

// Ensures that the ZVKSED extension (vector SM4 block cipher)
// is present, which depends on vector.
#define require_zvksed \
  do { \
    require_vector(true); \
    require_extension(EXT_ZVKSED); \
  } while (0)

// Ensures that the ZVKSH extension (vector SM3 hash) is present,
// which depends on vector.
#define require_zvksh \
  do { \
    require_vector(true); \
    require_extension(EXT_ZVKSH); \
  } while (0)

// Ensures that the vector instruction is not using a mask.
#define require_no_vmask  require(insn.v_vm() == 1)

// Ensures that the current VSEW is the given N number of bits.
#define require_vsew(N)  require(P.VU.vsew == (N))

// Ensures that an element group can fit in a register group. That is,
//    (LMUL * VLEN) <= EGW
#define require_egw_fits(EGW)  require((EGW) <= (P.VU.VLEN * P.VU.vflmul))

// Checks that the vector unit state (vtype and vl) can be interpreted
// as element groups with EEW=32, EGS=4 (four 32-bits elements per group),
// for an effective element group width of EGW=128 bits.
//
// Per the vector crypto specification, SEW is ignored. 'vl' and 'vstart'
// are interpreted as a number of EEW-wide elements. They must both
// be multiples of EGS (potentially 0).
#define require_element_groups_32x4 \
  do { \
    /* 'vstart' must be a multiple of EGS */ \
    const reg_t vstart = P.VU.vstart->read(); \
    require(vstart % 4 == 0); \
    /* 'vl' must be a multiple of EGS */ \
    const reg_t vl = P.VU.vl->read(); \
    require(vl % 4 == 0); \
  } while (0)

// Checks that the vector unit state (vtype and vl) can be interpreted
// as element groups with EEW=32, EGS=8 (eight 32-bits elements per group),
// for an effective element group width of EGW=256 bits.
//
// Per the vector crypto specification, SEW is ignored. 'vl' and 'vstart'
// are interpreted as a number of EEW-wide elements. They must both
// be multiples of EGS (potentially 0).
#define require_element_groups_32x8 \
  do { \
    /* 'vstart' must be a multiple of EGS */ \
    const reg_t vstart = P.VU.vstart->read(); \
    require(vstart % 8 == 0); \
    /* 'vl' must be a multiple of EGS */ \
    const reg_t vl = P.VU.vl->read(); \
    require(vl % 8 == 0); \
  } while (0)

// Checks that the vector unit state (vtype and vl) can be interpreted
// as element groups with EEW=64, EGS=4 (four 64-bits elements per group),
// for an effective element group width of EGW=128 bits.
//
// Per the vector crypto specification, SEW is ignored. 'vl' and 'vstart'
// are interpreted as a number of EEW-wide elements. They must both
// be multiples of EGS (potentially 0).
#define require_element_groups_64x4 \
  do { \
    /* 'vstart' must be a multiple of EGS */ \
    const reg_t vstart = P.VU.vstart->read(); \
    require(vstart % 4 == 0); \
    /* 'vl' must be a multiple of EGS */ \
    const reg_t vl = P.VU.vl->read(); \
    require(vl % 4 == 0); \
  } while (0)

//
// Loop Macros
//

// Extracts a 32b*4 element group as a EGU32x4_t variables at the given
// element group index, from register arguments 'vd' (by reference, mutable),
// 'vs1' and 'vs2' (constant, by value).
#define VV_VD_VS1_VS2_EGU32x4_PARAMS(VD_NUM, VS1_NUM, VS2_NUM, EG_IDX) \
  EGU32x4_t &vd = P.VU.elt_group<EGU32x4_t>((VD_NUM), (EG_IDX), true); \
  const EGU32x4_t vs1 = P.VU.elt_group<EGU32x4_t>((VS1_NUM), (EG_IDX)); \
  const EGU32x4_t vs2 = P.VU.elt_group<EGU32x4_t>((VS2_NUM), (EG_IDX))

// Extracts a 32b*8 element group as a EGU32x8_t variables at the given
// element group index, from register arguments 'vd' (by reference, mutable),
// 'vs1' and 'vs2' (constant, by value).
#define VV_VD_VS1_VS2_EGU32x8_PARAMS(VD_NUM, VS1_NUM, VS2_NUM, EG_IDX) \
  EGU32x8_t &vd = P.VU.elt_group<EGU32x8_t>((VD_NUM), (EG_IDX), true); \
  const EGU32x8_t vs1 = P.VU.elt_group<EGU32x8_t>((VS1_NUM), (EG_IDX)); \
  const EGU32x8_t vs2 = P.VU.elt_group<EGU32x8_t>((VS2_NUM), (EG_IDX))

// Extracts a 32b*4 element group as a EGU32x4_t variables at the given
// element group index, from register arguments 'vd' (by reference, mutable),
// and 'vs2' (constant, by value).
#define VV_VD_VS2_EGU32x4_PARAMS(VD_NUM, VS2_NUM, EG_IDX) \
  EGU32x4_t &vd = P.VU.elt_group<EGU32x4_t>((VD_NUM), (EG_IDX), true); \
  const EGU32x4_t vs2 = P.VU.elt_group<EGU32x4_t>((VS2_NUM), (EG_IDX))

// Extracts a 32b*8 element group as a EGU32x8_t variables at the given
// element group index, from register arguments 'vd' (by reference, mutable),
// and 'vs2' (constant, by value).
#define VV_VD_VS2_EGU32x8_PARAMS(VD_NUM, VS2_NUM, EG_IDX) \
  EGU32x8_t &vd = P.VU.elt_group<EGU32x8_t>((VD_NUM), (EG_IDX), true); \
  const EGU32x8_t vs2 = P.VU.elt_group<EGU32x8_t>((VS2_NUM), (EG_IDX))

// Extracts a 64b*4 element group as a EGU64x4_t variables at the given
// element group index, from register arguments 'vd' (by reference, mutable),
// 'vs1' and 'vs2' (constant, by value).
#define VV_VD_VS1_VS2_EGU64x4_PARAMS(VD_NUM, VS1_NUM, VS2_NUM, EG_IDX) \
  EGU64x4_t &vd = P.VU.elt_group<EGU64x4_t>((VD_NUM), (EG_IDX), true); \
  const EGU64x4_t vs1 = P.VU.elt_group<EGU64x4_t>((VS1_NUM), (EG_IDX)); \
  const EGU64x4_t vs2 = P.VU.elt_group<EGU64x4_t>((VS2_NUM), (EG_IDX))

//
// Loop Macros
//

// NOTES:
// - Each of the element-group loop macros DO contain an invocation
//   of the corresponding 'require_element_groups_<bits>x<#elements>;',
//   because the macro correctness requires proper VL/VSTART values.
// - Each of the loop macros named "_NOVM_" DO contain an invocation
//   of the 'require_no_vmask>;' macro. Those macros (all of them
//   at this time) do not support masking (i.e., no skipping
//   of elements/element groups is performed).

// Processes all 32b*4 element groups available in the vector register
// operands vd, vs1, and vs2.  This interprets the vectors as containing
// element groups of 4 uint32_t values (EGW=128, EEW=32, EGS=4), while
// *ignoring* the current SEW setting of the vector unit.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_32x4;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//  - While the name states "VD_VS1_VS2", many vector instructions
//    are specified as "op vd, vs2, vs1". This macro does not imply
//    a specific operand order and can be used with both "op vd, vs2, vs1"
//    and "op vd, vs1, vs2" instructions.
//
// Invokes two statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs1_num': register index of vs1
//   'vs2_num': register index of vs2
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//   'vd': EGU32x4_t reference, mutable,, content of the current
//         element group in the 'vd' vector register / register group.
//   'vs1': EGU32x4_t, content of the current element group
//          in the 'vs1' vector register / register group.
//   'vs2': EGU32x4_t, content of the current element group
//          in the 'vs2' vector register / register group.
//
#define VI_ZVK_VD_VS1_VS2_EGU32x4_NOVM_LOOP(PRELUDE, EG_BODY) \
  do { \
    require_element_groups_32x4; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs1_num = insn.rs1(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 4; \
    const reg_t vl_eg = P.VU.vl->read() / 4; \
    do { PRELUDE } while (0); \
    for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
      VV_VD_VS1_VS2_EGU32x4_PARAMS(vd_num, vs1_num, vs2_num, idx_eg); \
      do { EG_BODY } while (0); \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 32b*8 element groups available in the vector register
// operands vd, vs1, and vs2.  This interprets the vectors as containing
// element groups of 8 uint32_t values (EGW=256, EEW=32, EGS=8), while
// *ignoring* the current SEW setting of the vector unit.
//
// IMPORTANT
//  - This macro contains an invocation of the macro 'require_element_groups_32x8;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//  - While the name states "VD_VS1_VS2", many vector instructions
//    are specified as "op vd, vs2, vs1". This macro does not imply
//    a specific operand order and can be used with both "op vd, vs2, vs1"
//    and "op vd, vs1, vs2" instructions.
//
// Invokes two statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs1_num': register index of vs1
//   'vs2_num': register index of vs2
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//   'vd': EGU32x8_t reference, mutable,, content of the current
//         element group in the 'vd' vector register / register group.
//   'vs1': EGU32x8_t, content of the current element group
//          in the 'vs1' vector register / register group.
//   'vs2': EGU32x8_t, content of the current element group
//          in the 'vs2' vector register / register group.
//
#define VI_ZVK_VD_VS1_VS2_EGU32x8_NOVM_LOOP(PRELUDE, EG_BODY) \
  do { \
    require_element_groups_32x8;; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs1_num = insn.rs1(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 8; \
    const reg_t vl_eg = P.VU.vl->read() / 8; \
    do { PRELUDE } while (0); \
    for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
      VV_VD_VS1_VS2_EGU32x8_PARAMS(vd_num, vs1_num, vs2_num, idx_eg); \
      do { EG_BODY } while (0); \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 32b*4 element groups available in the vector register
// operands vd, vs1, and vs2.  This interprets the vectors as containing
// element groups of 4 uint32_t values (EGW=128, EEW=32, EGS=4), while
// *ignoring* the current SEW setting of the vector unit.
//
// Compared to VI_ZVK_VD_VS1_VS2_EGU32x4_NOVM_LOOP:
//  - this macro does NOT extract the element groups into EGU32x4_t
//    variables. It is intended for uses where there is a more natural
//    type to use (e.g., EGU8x16_t). The type should still be a 128 bits
//    wide type if extracted via 'P.VU.elt_group<Type>(...)'.
//  - this macro offers the additional PRELOOP code block argument,
//    that is executed once if the loop is going to be entered.
//    This is intended for use with "vector scalar" instructions where
//    we extract the first element group from one of the operands and
//    use it for all loop iterations.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_32x4;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//  - While the name states "VD_VS1_VS2", many vector instructions
//    are specified as "op vd, vs2, vs1". This macro does not imply
//    a specific operand order and can be used with both "op vd, vs2, vs1"
//    and "op vd, vs1, vs2" instructions.
//
// Invokes two statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - PRELOOP, invoked once IF there is at least one element group to process.
//    It is NOT placed in its own scope, variables declared in PRELOOP are
//    visible when EG_BODY executes.
//    Pass {} when there is no need for such a pre-loop block.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs1_num': register index of vs1
//   'vs2_num': register index of vs2
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//
#define VI_ZVK_VD_VS1_VS2_NOOPERANDS_PRELOOP_EGU32x4_NOVM_LOOP(PRELUDE, \
                                                               PRELOOP, \
                                                               EG_BODY) \
  do { \
    require_element_groups_32x4; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs1_num = insn.rs1(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 4; \
    const reg_t vl_eg = P.VU.vl->read() / 4; \
    do { PRELUDE } while (0); \
    if (vstart_eg < vl_eg) { \
      PRELOOP \
      for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
        do { EG_BODY } while (0); \
      } \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 32b*4 element groups available in the vector register
// operands vd and vs2.  This interprets the vectors as containing
// element groups of 4 uint32_t values (EGW=128, EEW=32, EGS=4), while
// *ignoring* the current SEW setting of the vector unit.
//
// Compared to VI_ZVK_VD_VS1_VS2_EGU32x4_NOVM_LOOP:
//  - this macro is meant to be used for "op vd, vs2" instructions,
//    whether vd is output only, or input and output.
//  - this macro does NOT extract the element groups into EGU32x4_t
//    variables. It is intended for uses where there is a more natural
//    type to use (e.g., EGU8x16_t). The type should still be a 128 bits
//    wide type if extracted via 'P.VU.elt_group<Type>(...)'.
//  - this macro offers the additional PRELOOP code block argument,
//    that is executed once if the loop is going to be entered.
//    This is intended for use with "vector scalar" instructions where
//    we extract the first element group from one of the operands and
//    use it for all loop iterations.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_32x4;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//  - While the name states "VD_VS1_VS2", many vector instructions
//    are specified as "op vd, vs2, vs1". This macro does not imply
//    a specific operand order and can be used with both "op vd, vs2, vs1"
//    and "op vd, vs1, vs2" instructions.
//
// Invokes three statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - PRELOOP, invoked once IF there is at least one element group to process.
//    It is NOT placed in its own scope, variables declared in PRELOOP are
//    visible when EG_BODY executes.
//    Pass {} when there is no need for such a pre-loop block.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs2_num': register index of vs2
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//
#define VI_ZVK_VD_VS2_NOOPERANDS_PRELOOP_EGU32x4_NOVM_LOOP(PRELUDE, \
                                                           PRELOOP, \
                                                           EG_BODY) \
  do { \
    require_element_groups_32x4; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 4; \
    const reg_t vl_eg = P.VU.vl->read() / 4; \
    do { PRELUDE } while (0); \
    if (vstart_eg < vl_eg) { \
      PRELOOP \
      for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
        do { EG_BODY } while (0); \
      } \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 32b*4 element groups available in the vector registers
// vd, vs2.  This interprets the vectors as containing element groups
// of 4 uint32_t values (EGW=128, EEW=32, EGS=4),
// *ignoring* the current SEW that applies to the vectors.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_32x4;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//
// Invokes two statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs2_num': register index of vs2
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//   'vd': EGU32x4_t reference, mutable,, content of the current
//         element group in the 'vd' vector register / register group.
//   'vs2': EGU32x4_t, content of the current element group
//          in the 'vs2' vector register / register group.
//
#define VI_ZVK_VD_VS2_EGU32x4_NOVM_LOOP(PRELUDE, EG_BODY) \
  do { \
    require_element_groups_32x4; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 4; \
    const reg_t vl_eg = P.VU.vl->read() / 4; \
    do { PRELUDE } while (0); \
    for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
      VV_VD_VS2_EGU32x4_PARAMS(vd_num, vs2_num, idx_eg); \
      do { EG_BODY } while (0); \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 32b*4 element groups available in the vector registers
// vd, vs2, given the 'zimm5' immediate.  This interprets the vectors as
// containing element groups of 4 uint32_t values (EGW=128, EEW=32, EGS=4),
// *ignoring* the current SEW that applies to the vectors.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_32x4;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//
// Invokes three statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - PRELOOP, invoked once IF there is at least one element group to process.
//    It is NOT placed in its own scope, variables declared in PRELOOP are
//    visible when EG_BODY executes.
//    Pass {} when there is no need for such a pre-loop block.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs2_num': register index of vs2
//   'zimm5': 5 bits unsigned immediate
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//   'vd': EGU32x4_t reference, mutable,, content of the current
//         element group in the 'vd' vector register / register group.
//   'vs2': EGU32x4_t, content of the current element group
//          in the 'vs2' vector register / register group.
//
#define VI_ZVK_VD_VS2_ZIMM5_EGU32x4_NOVM_LOOP(PRELUDE, PRELOOP, EG_BODY) \
  do { \
    require_element_groups_32x4; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t zimm5 = insn.v_zimm5(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 4; \
    const reg_t vl_eg = P.VU.vl->read() / 4; \
    do { PRELUDE } while (0); \
    if (vstart_eg < vl_eg) { \
      PRELOOP \
      for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
        VV_VD_VS2_EGU32x4_PARAMS(vd_num, vs2_num, idx_eg); \
        do { EG_BODY } while (0); \
      } \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 32b*8 element groups available in the vector registers
// vd, vs2, given the 'zimm5' immediate.  This interprets the vectors as
// containing element groups of 8 uint32_t values (EGW=256, EEW=32, EGS=8),
// *ignoring* the current SEW that applies to the vectors.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_32x8;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//
// Invokes three statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - PRELOOP, invoked once IF there is at least one element group to process.
//    It is NOT placed in its own scope, variables declared in PRELOOP are
//    visible when EG_BODY executes.
//    Pass {} when there is no need for such a pre-loop block.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs2_num': register index of vs2
//   'zimm5': unsigned 5 bits immediate
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//   'vd': EGU32x8_t reference, mutable,, content of the current
//         element group in the 'vd' vector register / register group.
//   'vs2': EGU32x8_t, content of the current element group
//          in the 'vs2' vector register / register group.
//
#define VI_ZVK_VD_VS2_ZIMM5_EGU32x8_NOVM_LOOP(PRELUDE, PRELOOP, EG_BODY) \
  do { \
    require_element_groups_32x8; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t zimm5 = insn.v_zimm5(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 8; \
    const reg_t vl_eg = P.VU.vl->read() / 8; \
    do { PRELUDE } while (0); \
    if (vstart_eg < vl_eg) { \
      PRELOOP \
      for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
        VV_VD_VS2_EGU32x8_PARAMS(vd_num, vs2_num, idx_eg); \
        do { EG_BODY } while (0); \
      } \
    } \
    P.VU.vstart->write(0); \
  } while (0)

// Processes all 64b*4 element groups available in the vector registers
// vd, vs1, and vs2.  This interprets the vectors as containing element groups
// of 4 uint64_t values (EGW=128, EEW=64, EGS=4), *ignoring* the current
// SEW that applies to the vectors.
//
// IMPORTANT
//  - This macro contains an invocation of 'require_element_groups_64x4;',
//    since the "loop" macro correctness depends on invariants that
//    are checked by the "require" macro.
//  - This macro does not support masking, and contains an invocation
//    of 'require_no_vmask;'.
//  - While the name states "VD_VS1_VS2", many vector instructions
//    are specified as "op vd, vs2, vs1". This macro does not imply
//    a specific operand order and can be used with both "op vd, vs2, vs1"
//    and "op vd, vs1, vs2" instructions.
//
// Invokes two statement blocks:
//  - PRELUDE, invoked once, before any element group. It is executed even
//    if the vector is empty. It is placed in a "do { } while (0);", hence
//    any variable declared there is not visible outside.
//  - EG_BODY, once per element group. It is placed in a "do { } while (0);",
//    hence any variable declared there is not visible outside.
//
// Declares the following variables available for use in both statement blocks:
//   'vd_num': register index of vd
//   'vs1_num': register index of vs1
//   'vs2_num': register index of vs2
//   'vstart_eg': index of the first element group, *in EG units*
//   'vl_eg': length of the vector, *in EG units*
//
// The following variables are available in the EG_BODY block:
//   'idx_eg': index of the current element group.
//   'vd': EGU64x4_t reference, content of the current element group
//         in the 'vd' vector register / vector register group.
//   'vs1': EGU64x4_t, content of the current element group
//         in the 'vs1' vector register / vector register group.
//   'vs2': EGU64x4_t, content of the current element group
//         in the 'vs2' vector register / vector register group.
#define VI_ZVK_VD_VS1_VS2_EGU64x4_NOVM_LOOP(PRELUDE, EG_BODY) \
  do { \
    require_element_groups_64x4; \
    require_no_vmask; \
    const reg_t vsew = P.VU.vsew; \
    const reg_t vd_num = insn.rd(); \
    const reg_t vs1_num = insn.rs1(); \
    const reg_t vs2_num = insn.rs2(); \
    const reg_t vstart_eg = P.VU.vstart->read() / 4; \
    const reg_t vl_eg = P.VU.vl->read() / 4; \
    do { PRELUDE } while (0); \
    for (reg_t idx_eg = vstart_eg; idx_eg < vl_eg; ++idx_eg) { \
      VV_VD_VS1_VS2_EGU64x4_PARAMS(vd_num, vs1_num, vs2_num, idx_eg); \
      do { EG_BODY } while (0); \
    } \
    P.VU.vstart->write(0); \
  } while (0)

//
// Element Group Manipulation Macros
//

// Extracts 4 uint32_t words from the input EGU32x4_t value
// into the (mutable) variables named by the W arguments, provided in
// "Little Endian" (LE) order, i.e., from the least significant (W0)
// to the most significant (W3).
#define EXTRACT_EGU32x4_WORDS_LE(X, W0, W1, W2, W3) \
  uint32_t W0 = (X)[0]; \
  uint32_t W1 = (X)[1]; \
  uint32_t W2 = (X)[2]; \
  uint32_t W3 = (X)[3]; \
  (void)(0)

// Sets the elements words of given EGU32x4_t variable 'X' to
// the given 4 uint32_t values privided in "Little Endian" (LE)
// order, i.e., from the least significant (W0) to the most
// significant (W3).
#define SET_EGU32x4_LE(X, W0, W1, W2, W3) \
  do { \
    (X)[0] = (W0); \
    (X)[1] = (W1); \
    (X)[2] = (W2); \
    (X)[3] = (W3); \
  } while (0)

// Extracts 4 uint32_t words from the input EGU32x4_t value
// into the (mutable) variables named by the W arguments, provided in
// "Big Endian" (BE) order, i.e., from the most significant (W3)
// to the least significant (W0).
#define EXTRACT_EGU32x4_WORDS_BE(X, W3, W2, W1, W0) \
  uint32_t W0 = (X)[0]; \
  uint32_t W1 = (X)[1]; \
  uint32_t W2 = (X)[2]; \
  uint32_t W3 = (X)[3]; \
  (void)(0)

// Sets the elements words of given EGU32x4_t variable 'X' to
// the given 4 uint32_t values privided in "Big Endian" (BE)
// order, i.e., from the most significant (W3) to the least
// significant (W0).
#define SET_EGU32x4_BE(X, W3, W2, W1, W0) \
  do { \
    (X)[0] = (W0); \
    (X)[1] = (W1); \
    (X)[2] = (W2); \
    (X)[3] = (W3); \
  } while (0)

// Extracts 8 uint32_t words from the input EGU32x8_t value
// into the (mutable) variables named by the W arguments, provided in
// "Big Endian" (BE) order, i.e., from the most significant (W7)
// to the least significant (W0). Each of the words is byte-swapped,
// from a big-endian representation in the EGU32x8_t to a native/little-endian
// ordering in the variables.
#define EXTRACT_EGU32x8_WORDS_BE_BSWAP(X, W7, W6, W5, W4, W3, W2, W1, W0) \
  uint32_t W0 = BSWAP32((X)[0]); \
  uint32_t W1 = BSWAP32((X)[1]); \
  uint32_t W2 = BSWAP32((X)[2]); \
  uint32_t W3 = BSWAP32((X)[3]); \
  uint32_t W4 = BSWAP32((X)[4]); \
  uint32_t W5 = BSWAP32((X)[5]); \
  uint32_t W6 = BSWAP32((X)[6]); \
  uint32_t W7 = BSWAP32((X)[7]); \
  (void)(0)

// Sets the elements words of given EGU32x8_t variable 'X' to
// the given 8 uint32_t values privided in "Big Endian" (BE)
// order, i.e., from the most significant (W7) to the least
// significant (W0). Each of the words is byte-swapped,
// from a native/little-endian ordering in the variables to
// a big-endian representation in the EGU32x8_t.
#define SET_EGU32x8_WORDS_BE_BSWAP(X, W7, W6, W5, W4, W3, W2, W1, W0) \
  do { \
    (X)[0] = BSWAP32(W0); \
    (X)[1] = BSWAP32(W1); \
    (X)[2] = BSWAP32(W2); \
    (X)[3] = BSWAP32(W3); \
    (X)[4] = BSWAP32(W4); \
    (X)[5] = BSWAP32(W5); \
    (X)[6] = BSWAP32(W6); \
    (X)[7] = BSWAP32(W7); \
  } while (0)

// Extracts 4 uint64_t words from the input EGU64x4_t value
// into the (mutable) variables named by the W arguments, provided in
// "Big Endian" (BE) order, i.e., from the most significant (W3)
// to the least significant (W0).
#define EXTRACT_EGU64x4_WORDS_BE(X, W3, W2, W1, W0) \
  uint64_t W0 = (X)[0]; \
  uint64_t W1 = (X)[1]; \
  uint64_t W2 = (X)[2]; \
  uint64_t W3 = (X)[3]; \
  (void)(0)

// Sets the elements words of given EGU64x4_t variable 'X' to
// the given 4 uint64_t values privided in "Big Endian" (BE)
// order, i.e., from the most significant (W3) to the least
// significant (W0).
#define SET_EGU64x4_BE(X, W3, W2, W1, W0) \
  do { \
    (X)[0] = (W0); \
    (X)[1] = (W1); \
    (X)[2] = (W2); \
    (X)[3] = (W3); \
  } while (0)

// Copies a EGU8x16_t value from 'SRC' into 'DST'.
#define EGU8x16_COPY(DST, SRC) \
  for (std::size_t bidx = 0; bidx < 16; ++bidx) { \
    (DST)[bidx] = (SRC)[bidx]; \
  }

// Performs  "MUT_A ^= CONST_B;", i.e., xor of the bytes
// in A (mutated) with the bytes in B (unchanged).
#define EGU8x16_XOREQ(MUT_A, CONST_B) \
  for (std::size_t bidx = 0; bidx < 16; ++bidx) { \
    (MUT_A)[bidx] ^= (CONST_B)[bidx]; \
  }

// Performs  "MUT_A ^= CONST_B;", i.e., xor of the bytes
// in A (mutated) with the bytes in B (unchanged).
#define EGU32x4_XOREQ(MUT_A, CONST_B) \
  for (std::size_t idx = 0; idx < 4; ++idx) { \
    (MUT_A)[idx] ^= (CONST_B)[idx]; \
  }

// Performs  "DST = A ^ B;", i.e., DST (overwritten) receives
// the xor of the bytes in A and B (both unchanged).
#define EGU8x16_XOR(DST, A, B) \
  for (std::size_t bidx = 0; bidx < 16; ++bidx) { \
    (DST)[bidx] = (A)[bidx] ^ (B)[bidx]; \
  }

// Performs  "DST = A ^ B;", i.e., DST (overwritten) receives
// the xor of the bytes in A and B (both unchanged).
#define EGU32x4_XOR(DST, A, B) \
  do { \
    static_assert(std::is_same<EGU32x4_t, decltype(A)>::value); \
    static_assert(std::is_same<EGU32x4_t, decltype(B)>::value); \
    static_assert(std::is_same<EGU32x4_t, decltype(DST)>::value); \
    for (std::size_t idx = 0; idx < 4; ++idx) { \
      (DST)[idx] = (A)[idx] ^ (B)[idx]; \
    } \
  } while (0)

//
// Byte manipulation routines.
//

// Given an unsigned integer value 'X' and a byte index,
// returns a uint8_t value for the byte at the given index.
#define EXTRACT_U8(X, BYTE_IDX) ((uint8_t)((X) >> (BYTE_IDX * 8)))

// Returns a uint32_t value constructed from the 4 bytes (uint8_t)
// provided in "Little Endian" (LE) order, i.e., from least significant (B0)
// to most significant (B3).
#define U32_FROM_U8_LE(B0, B1, B2, B3) \
  (((uint32_t)(B0)) <<  0 | \
   ((uint32_t)(B1)) <<  8 | \
   ((uint32_t)(B2)) << 16 | \
   ((uint32_t)(B3)) << 24)

//
// Common bit manipulations logic.
//

// Form a 64 bit integer with bit X set
#define BIT(X) (1ULL << (X))

// Reverse the order of bits within bytes of a word.
// This is used to match the data interpretation in NIST SP 800-38D
// a.k.a the GCM specification.
#define BREV8_32(X) \
  do { \
    (X) = (((X) & 0x55555555) << 1) | (((X) & 0xaaaaaaaa) >> 1); \
    (X) = (((X) & 0x33333333) << 2) | (((X) & 0xcccccccc) >> 2); \
    (X) = (((X) & 0x0f0f0f0f) << 4) | (((X) & 0xf0f0f0f0) >> 4); \
  } while (0)

// ROR32: rotates right a uint32_t value by N bits.
//   uint32_t ROR32(uint32_t X, unsigned int N);
//
// The static_assert is there to force N to be a compile-time
// constant in [1, 31]. For runtime 'N' values, logic has to be present
// to protect against Undefined Behavior due to shift values <= 0
// or >= 32.
#define ROR32(X, N) \
  ([]{ static_assert(1 <= (N) && (N) <= 31); }, \
   (((X) >> (N)) | ((X) << (32 - (N)))))

// ROR64: rotates right a uint64_t value by N bits.
//   uint64_t ROR64(uint64_t X, unsigned int N);
//
// The static_assert is there to force N to be a compile-time
// constant in [1, 63]. For runtime 'N' values, logic has to be present
// to protect against Undefined Behavior due to shift values <= 0
// or >= 32.
#define ROR64(X, N) \
  ([]{ static_assert(1 <= (N) && (N) <= 63); }, \
   (((X) >> (N)) | ((X) << (64 - (N)))))

// ROL32: rotates left a uint32_t value by N bits.
//   uint32_t ROL32(uint32_t X, unsigned int N);
//
// The static_assert is there to force N to be a compile-time
// constant in [1, 31]. For runtime 'N' values, logic has to be present
// to protect against Undefined Behavior due to shift values <= 0
// or >= 32.
#define ROL32(X, N) \
  ([]{ static_assert(1 <= (N) && (N) <= 31); }, \
   (((X) << (N)) | ((X) >> (32 - (N)))))

//
// Element Group Bit Manipulation Macros
//

// Performs bit reversal in a EGU32x4_t group.
#define EGU32x4_BREV8(X) \
  for (std::size_t bidx = 0; bidx < 4; ++bidx) { \
    BREV8_32((X)[bidx]); \
  }

// Checks if a given bit is set within a EGU32x4_t group.
// Assumes LE ordering.
#define EGU32x4_ISSET(X, BIDX) \
  (((X)[(BIDX) / 32] & BIT((BIDX) % 32)) != 0)

// Shfts a EGU32x4_t group left by one bit.
//
// Since the entire 128 bit value is shifted we need to handle carry bits.
// In order to limit the amount of carry check logic the elements are copied to
// a 64 bit temporary variable.
#define EGU32x4_LSHIFT(X) \
  do { \
    uint64_t dword; \
    dword = ((uint64_t)(X)[3]) << 32; \
    dword |= X[2]; \
    dword <<= 1; \
    if (X[1] & BIT(31)) \
        dword |= BIT(0); \
    X[2] = dword & UINT32_MAX; \
    X[3] = dword >> 32; \
    dword = ((uint64_t)(X)[1]) << 32; \
    dword |= X[0]; \
    dword <<= 1; \
    X[0] = dword & UINT32_MAX; \
    X[1] = dword >> 32; \
  } while (0)

#endif  // RISCV_ZVK_EXT_MACROS_H_
