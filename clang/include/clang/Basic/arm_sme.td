//===--- arm_sme.td - ARM SME compiler interface ------------------------===//
//
//  Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
//  See https://llvm.org/LICENSE.txt for license information.
//  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TableGen definitions from which the ARM SME header
//  file will be generated.  See:
//
//      https://developer.arm.com/architectures/system-architectures/software-standards/acle
//
//===----------------------------------------------------------------------===//

include "arm_sve_sme_incl.td"

let SVETargetGuard = InvalidMode in {

////////////////////////////////////////////////////////////////////////////////
// Loads

multiclass ZALoad<string n_suffix, string t, string i_prefix, list<ImmCheck> ch> {
  let SMETargetGuard = "sme" in {
    def NAME # _H : MInst<"svld1_hor_" # n_suffix, "vimPQ", t,
                          [IsLoad, IsOverloadNone, IsStreaming, IsInOutZA],
                          MemEltTyDefault, i_prefix # "_horiz", ch>;

    def NAME # _H_VNUM : MInst<"svld1_hor_vnum_" # n_suffix, "vimPQl", t,
                               [IsLoad, IsOverloadNone, IsStreaming, IsInOutZA],
                               MemEltTyDefault, i_prefix # "_horiz", ch>;

    def NAME # _V : MInst<"svld1_ver_" # n_suffix, "vimPQ", t,
                          [IsLoad, IsOverloadNone, IsStreaming, IsInOutZA],
                          MemEltTyDefault, i_prefix # "_vert", ch>;

    def NAME # _V_VNUM : MInst<"svld1_ver_vnum_" # n_suffix, "vimPQl", t,
                               [IsLoad, IsOverloadNone, IsStreaming, IsInOutZA],
                               MemEltTyDefault, i_prefix # "_vert", ch>;
  }
}

defm SVLD1_ZA8 : ZALoad<"za8", "c", "aarch64_sme_ld1b", [ImmCheck<0, ImmCheck0_0>]>;
defm SVLD1_ZA16 : ZALoad<"za16", "s", "aarch64_sme_ld1h", [ImmCheck<0, ImmCheck0_1>]>;
defm SVLD1_ZA32 : ZALoad<"za32", "i", "aarch64_sme_ld1w", [ImmCheck<0, ImmCheck0_3>]>;
defm SVLD1_ZA64 : ZALoad<"za64", "l", "aarch64_sme_ld1d", [ImmCheck<0, ImmCheck0_7>]>;
defm SVLD1_ZA128 : ZALoad<"za128", "q", "aarch64_sme_ld1q", [ImmCheck<0, ImmCheck0_15>]>;

let SMETargetGuard = "sme" in {
def SVLDR_VNUM_ZA : MInst<"svldr_vnum_za", "vmQl", "",
                          [IsOverloadNone, IsStreamingCompatible, IsInOutZA],
                          MemEltTyDefault, "aarch64_sme_ldr">;

def SVLDR_ZA : MInst<"svldr_za", "vmQ", "",
                          [IsOverloadNone, IsStreamingCompatible, IsInOutZA],
                          MemEltTyDefault, "aarch64_sme_ldr", []>;
}

////////////////////////////////////////////////////////////////////////////////
// Stores

multiclass ZAStore<string n_suffix, string t, string i_prefix, list<ImmCheck> ch> {
  let SMETargetGuard = "sme" in {
    def NAME # _H : MInst<"svst1_hor_" # n_suffix, "vimP%", t,
                          [IsStore, IsOverloadNone, IsStreaming, IsInZA],
                          MemEltTyDefault, i_prefix # "_horiz", ch>;

    def NAME # _H_VNUM : MInst<"svst1_hor_vnum_" # n_suffix, "vimP%l", t,
                               [IsStore, IsOverloadNone, IsStreaming, IsInZA],
                               MemEltTyDefault, i_prefix # "_horiz", ch>;

    def NAME # _V : MInst<"svst1_ver_" # n_suffix, "vimP%", t,
                          [IsStore, IsOverloadNone, IsStreaming, IsInZA],
                          MemEltTyDefault, i_prefix # "_vert", ch>;

    def NAME # _V_VNUM : MInst<"svst1_ver_vnum_" # n_suffix, "vimP%l", t,
                               [IsStore, IsOverloadNone, IsStreaming, IsInZA],
                               MemEltTyDefault, i_prefix # "_vert", ch>;
  }
}

defm SVST1_ZA8 : ZAStore<"za8", "c", "aarch64_sme_st1b", [ImmCheck<0, ImmCheck0_0>]>;
defm SVST1_ZA16 : ZAStore<"za16", "s", "aarch64_sme_st1h", [ImmCheck<0, ImmCheck0_1>]>;
defm SVST1_ZA32 : ZAStore<"za32", "i", "aarch64_sme_st1w", [ImmCheck<0, ImmCheck0_3>]>;
defm SVST1_ZA64 : ZAStore<"za64", "l", "aarch64_sme_st1d", [ImmCheck<0, ImmCheck0_7>]>;
defm SVST1_ZA128 : ZAStore<"za128", "q", "aarch64_sme_st1q", [ImmCheck<0, ImmCheck0_15>]>;

let SMETargetGuard = "sme" in {
def SVSTR_VNUM_ZA : MInst<"svstr_vnum_za", "vm%l", "",
                          [IsOverloadNone, IsStreamingCompatible, IsInZA],
                          MemEltTyDefault, "aarch64_sme_str">;

def SVSTR_ZA : MInst<"svstr_za", "vm%", "",
                      [IsOverloadNone, IsStreamingCompatible, IsInZA],
                      MemEltTyDefault, "aarch64_sme_str", []>;
}

////////////////////////////////////////////////////////////////////////////////
// Read horizontal/vertical ZA slices

multiclass ZARead<string n_suffix, string t, string i_prefix, list<ImmCheck> ch> {
  let SMETargetGuard = "sme" in {
    def NAME # _H : SInst<"svread_hor_" # n_suffix # "[_{d}]", "ddPim", t,
                          MergeOp1, i_prefix # "_horiz",
                          [IsReadZA, IsStreaming, IsInZA], ch>;

    def NAME # _V : SInst<"svread_ver_" # n_suffix # "[_{d}]", "ddPim", t,
                          MergeOp1, i_prefix # "_vert",
                          [IsReadZA, IsStreaming, IsInZA], ch>;
  }
}

defm SVREAD_ZA8 : ZARead<"za8", "cUc", "aarch64_sme_read", [ImmCheck<2, ImmCheck0_0>]>;
defm SVREAD_ZA16 : ZARead<"za16", "sUshb", "aarch64_sme_read", [ImmCheck<2, ImmCheck0_1>]>;
defm SVREAD_ZA32 : ZARead<"za32", "iUif", "aarch64_sme_read", [ImmCheck<2, ImmCheck0_3>]>;
defm SVREAD_ZA64 : ZARead<"za64", "lUld", "aarch64_sme_read", [ImmCheck<2, ImmCheck0_7>]>;
defm SVREAD_ZA128 : ZARead<"za128", "csilUcUsUiUlhbfd", "aarch64_sme_readq", [ImmCheck<2, ImmCheck0_15>]>;

////////////////////////////////////////////////////////////////////////////////
// Write horizontal/vertical ZA slices

multiclass ZAWrite<string n_suffix, string t, string i_prefix, list<ImmCheck> ch> {
  let SMETargetGuard = "sme" in {
    def NAME # _H : SInst<"svwrite_hor_" # n_suffix # "[_{d}]", "vimPd", t,
                          MergeOp1, i_prefix # "_horiz",
                          [IsWriteZA, IsStreaming, IsInOutZA], ch>;

    def NAME # _V : SInst<"svwrite_ver_" # n_suffix # "[_{d}]", "vimPd", t,
                          MergeOp1, i_prefix # "_vert",
                          [IsWriteZA, IsStreaming, IsInOutZA], ch>;
  }
}

defm SVWRITE_ZA8 : ZAWrite<"za8", "cUc", "aarch64_sme_write", [ImmCheck<0, ImmCheck0_0>]>;
defm SVWRITE_ZA16 : ZAWrite<"za16", "sUshb", "aarch64_sme_write", [ImmCheck<0, ImmCheck0_1>]>;
defm SVWRITE_ZA32 : ZAWrite<"za32", "iUif", "aarch64_sme_write", [ImmCheck<0, ImmCheck0_3>]>;
defm SVWRITE_ZA64 : ZAWrite<"za64", "lUld", "aarch64_sme_write", [ImmCheck<0, ImmCheck0_7>]>;
defm SVWRITE_ZA128 : ZAWrite<"za128", "csilUcUsUiUlhbfd", "aarch64_sme_writeq", [ImmCheck<0, ImmCheck0_15>]>;

////////////////////////////////////////////////////////////////////////////////
// SME - Zero

let SMETargetGuard = "sme" in {
  def SVZERO_MASK_ZA : SInst<"svzero_mask_za", "vi", "", MergeNone, "aarch64_sme_zero",
                             [IsOverloadNone, IsStreamingCompatible, IsInOutZA],
                             [ImmCheck<0, ImmCheck0_255>]>;
  def SVZERO_ZA      : SInst<"svzero_za", "v", "", MergeNone, "aarch64_sme_zero",
                             [IsOverloadNone, IsStreamingCompatible, IsOutZA]>;
}

////////////////////////////////////////////////////////////////////////////////
// SME - Counting elements in a streaming vector

multiclass ZACount<string n_suffix> {
  let SMETargetGuard = "sme" in {
    def NAME : SInst<"sv" # n_suffix, "nv", "", MergeNone,
                      "aarch64_sme_" # n_suffix,
                      [IsOverloadNone, IsStreamingCompatible]>;
  }
}

defm SVCNTSB : ZACount<"cntsb">;
defm SVCNTSH : ZACount<"cntsh">;
defm SVCNTSW : ZACount<"cntsw">;
defm SVCNTSD : ZACount<"cntsd">;

////////////////////////////////////////////////////////////////////////////////
// SME - ADDHA/ADDVA

multiclass ZAAdd<string n_suffix> {
  let SMETargetGuard = "sme" in {
    def NAME # _ZA32: SInst<"sv" # n_suffix # "_za32[_{d}]", "viPPd", "iUi", MergeOp1,
                      "aarch64_sme_" # n_suffix, [IsStreaming, IsInOutZA],
                      [ImmCheck<0, ImmCheck0_3>]>;
  }

  let SMETargetGuard = "sme-i16i64" in {
    def NAME # _ZA64: SInst<"sv" # n_suffix # "_za64[_{d}]", "viPPd", "lUl", MergeOp1,
                     "aarch64_sme_" # n_suffix, [IsStreaming, IsInOutZA],
                     [ImmCheck<0, ImmCheck0_7>]>;
  }
}

defm SVADDHA : ZAAdd<"addha">;
defm SVADDVA : ZAAdd<"addva">;

////////////////////////////////////////////////////////////////////////////////
// SME - SMOPA, SMOPS, UMOPA, UMOPS

multiclass ZAIntOuterProd<string n_suffix1, string n_suffix2> {
  let SMETargetGuard = "sme" in {
    def NAME # _ZA32_B: SInst<"sv" # n_suffix2 # "_za32[_{d}]",
                              "viPPdd", !cond(!eq(n_suffix1, "s") : "", true: "U") # "c",
                              MergeOp1, "aarch64_sme_" # n_suffix1 # n_suffix2 # "_wide",
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_3>]>;
  }

  let SMETargetGuard = "sme-i16i64" in {
    def NAME # _ZA64_H: SInst<"sv" # n_suffix2 # "_za64[_{d}]",
                              "viPPdd", !cond(!eq(n_suffix1, "s") : "", true: "U") # "s",
                              MergeOp1, "aarch64_sme_" # n_suffix1 # n_suffix2 # "_wide",
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_7>]>;
  }
}

defm SVSMOPA : ZAIntOuterProd<"s", "mopa">;
defm SVSMOPS : ZAIntOuterProd<"s", "mops">;
defm SVUMOPA : ZAIntOuterProd<"u", "mopa">;
defm SVUMOPS : ZAIntOuterProd<"u", "mops">;

////////////////////////////////////////////////////////////////////////////////
// SME - SUMOPA, SUMOPS, USMOPA, USMOPS

multiclass ZAIntOuterProdMixedSigns<string n_suffix1, string n_suffix2> {
  let SMETargetGuard = "sme" in {
    def NAME # _ZA32_B: SInst<"sv" # n_suffix1 # n_suffix2 # "_za32[_{d}]",
                              "viPPd" # !cond(!eq(n_suffix1, "su") : "u", true: "x"),
                              !cond(!eq(n_suffix1, "su") : "", true: "U") # "c",
                              MergeOp1, "aarch64_sme_" # n_suffix1 # n_suffix2 # "_wide",
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_3>]>;
  }

  let SMETargetGuard = "sme-i16i64" in {
    def NAME # _ZA64_H: SInst<"sv" # n_suffix1 # n_suffix2 # "_za64[_{d}]",
                              "viPPd" # !cond(!eq(n_suffix1, "su") : "u", true: "x"),
                              !cond(!eq(n_suffix1, "su") : "", true: "U") # "s",
                              MergeOp1, "aarch64_sme_" # n_suffix1 # n_suffix2 # "_wide",
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_7>]>;
  }
}

defm SVSUMOPA : ZAIntOuterProdMixedSigns<"su", "mopa">;
defm SVSUMOPS : ZAIntOuterProdMixedSigns<"su", "mops">;
defm SVUSMOPA : ZAIntOuterProdMixedSigns<"us", "mopa">;
defm SVUSMOPS : ZAIntOuterProdMixedSigns<"us", "mops">;

////////////////////////////////////////////////////////////////////////////////
// SME - FMOPA, FMOPS

multiclass ZAFPOuterProd<string n_suffix> {
  let SMETargetGuard = "sme" in {
    def NAME # _ZA32_B: SInst<"sv" # n_suffix # "_za32[_{d}]", "viPPdd", "h",
                              MergeOp1, "aarch64_sme_" # n_suffix # "_wide",
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_3>]>;

    def NAME # _ZA32_H: SInst<"sv" # n_suffix # "_za32[_{d}]", "viPPdd", "b",
                              MergeOp1, "aarch64_sme_" # n_suffix # "_wide",
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_3>]>;

    def NAME # _ZA32_S: SInst<"sv" # n_suffix # "_za32[_{d}]", "viPPdd", "f",
                              MergeOp1, "aarch64_sme_" # n_suffix,
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_3>]>;
  }

  let SMETargetGuard = "sme-f64f64" in {
    def NAME # _ZA64_D: SInst<"sv" # n_suffix # "_za64[_{d}]", "viPPdd", "d",
                              MergeOp1, "aarch64_sme_" # n_suffix,
                              [IsStreaming, IsInOutZA],
                              [ImmCheck<0, ImmCheck0_7>]>;
  }
}

defm SVMOPA : ZAFPOuterProd<"mopa">;
defm SVMOPS : ZAFPOuterProd<"mops">;

////////////////////////////////////////////////////////////////////////////////
// SME2 - ADD, SUB

multiclass ZAAddSub<string n_suffix> {
  let SMETargetGuard = "sme2" in {
    def NAME # _WRITE_SINGLE_ZA32_VG1X2_I32 : Inst<"sv" # n_suffix # "_write[_single]_za32[_{d}]_vg1x2", "vm2d", "iUi", MergeNone, "aarch64_sme_" # n_suffix # "_write_single_za_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _WRITE_SINGLE_ZA32_VG1X4_I32 : Inst<"sv" # n_suffix # "_write[_single]_za32[_{d}]_vg1x4", "vm4d", "iUi", MergeNone, "aarch64_sme_" # n_suffix # "_write_single_za_vg1x4", [IsStreaming, IsInOutZA], []>;

    def NAME # _WRITE_ZA32_VG1X2_I32 : Inst<"sv" # n_suffix # "_write_za32[_{d}]_vg1x2", "vm22", "iUi", MergeNone, "aarch64_sme_" # n_suffix # "_write_za_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _WRITE_ZA32_VG1X4_I32 : Inst<"sv" # n_suffix # "_write_za32[_{d}]_vg1x4", "vm44", "iUi", MergeNone, "aarch64_sme_" # n_suffix # "_write_za_vg1x4", [IsStreaming, IsInOutZA], []>;

    def NAME # _ZA32_VG1x2_I32 : Inst<"sv" # n_suffix # "_za32[_{d}]_vg1x2", "vm2", "iUif", MergeNone, "aarch64_sme_" # n_suffix # "_za32_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _ZA32_VG1X4_I32 : Inst<"sv" # n_suffix # "_za32[_{d}]_vg1x4", "vm4", "iUif", MergeNone, "aarch64_sme_" # n_suffix # "_za32_vg1x4", [IsStreaming, IsInOutZA], []>;
  }

  let SMETargetGuard = "sme2,sme-i16i64" in {
    def NAME # _WRITE_SINGLE_ZA64_VG1X2_I64 : Inst<"sv" # n_suffix # "_write[_single]_za64[_{d}]_vg1x2", "vm2d", "lUl", MergeNone, "aarch64_sme_" # n_suffix # "_write_single_za_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _WRITE_SINGLE_ZA64_VG1X4_I64 : Inst<"sv" # n_suffix # "_write[_single]_za64[_{d}]_vg1x4", "vm4d", "lUl", MergeNone, "aarch64_sme_" # n_suffix # "_write_single_za_vg1x4", [IsStreaming, IsInOutZA], []>;

    def NAME # _WRITE_ZA64_VG1x2_I64 : Inst<"sv" # n_suffix # "_write_za64[_{d}]_vg1x2", "vm22", "lUl", MergeNone, "aarch64_sme_" # n_suffix # "_write_za_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _WRITE_ZA64_VG1x4_I64 : Inst<"sv" # n_suffix # "_write_za64[_{d}]_vg1x4", "vm44", "lUl", MergeNone, "aarch64_sme_" # n_suffix # "_write_za_vg1x4", [IsStreaming, IsInOutZA], []>;

    def NAME # _ZA64_VG1X2_I64 : Inst<"sv" # n_suffix # "_za64[_{d}]_vg1x2", "vm2", "lUl", MergeNone, "aarch64_sme_" # n_suffix # "_za64_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _ZA64_VG1X4_I64 : Inst<"sv" # n_suffix # "_za64[_{d}]_vg1x4", "vm4", "lUl", MergeNone, "aarch64_sme_" # n_suffix # "_za64_vg1x4", [IsStreaming, IsInOutZA], []>;
  }

  let SMETargetGuard = "sme2,sme-f64f64" in {
    def NAME # _ZA64_VG1X2_F64 : Inst<"sv" # n_suffix # "_za64[_{d}]_vg1x2", "vm2", "d", MergeNone, "aarch64_sme_" # n_suffix # "_za64_vg1x2", [IsStreaming, IsInOutZA], []>;
    def NAME # _ZA64_VG1X4_F64 : Inst<"sv" # n_suffix # "_za64[_{d}]_vg1x4", "vm4", "d", MergeNone, "aarch64_sme_" # n_suffix # "_za64_vg1x4", [IsStreaming, IsInOutZA], []>;
  }
}

defm SVADD : ZAAddSub<"add">;
defm SVSUB : ZAAddSub<"sub">;

//
// lookup table expand four contiguous registers
//
let SMETargetGuard = "sme2" in {
  def SVLUTI2_LANE_ZT_X4 : Inst<"svluti2_lane_zt_{d}_x4", "4.di[i", "cUcsUsiUibhf", MergeNone, "aarch64_sme_luti2_lane_zt_x4", [IsStreaming, IsInOutZA], [ImmCheck<0, ImmCheck0_0>, ImmCheck<2, ImmCheck0_3>]>;
  def SVLUTI4_LANE_ZT_X4 : Inst<"svluti4_lane_zt_{d}_x4", "4.di[i", "sUsiUibhf", MergeNone, "aarch64_sme_luti4_lane_zt_x4", [IsStreaming, IsInOutZA], [ImmCheck<0, ImmCheck0_0>, ImmCheck<2, ImmCheck0_1>]>;
}

//
// lookup table expand one register
//
let SMETargetGuard = "sme2" in {
  def SVLUTI2_LANE_ZT : Inst<"svluti2_lane_zt_{d}", "di[i", "cUcsUsiUibhf", MergeNone, "aarch64_sme_luti2_lane_zt", [IsStreaming, IsInOutZA], [ImmCheck<0, ImmCheck0_0>, ImmCheck<2, ImmCheck0_15>]>;
  def SVLUTI4_LANE_ZT : Inst<"svluti4_lane_zt_{d}", "di[i", "cUcsUsiUibhf", MergeNone, "aarch64_sme_luti4_lane_zt", [IsStreaming, IsInOutZA], [ImmCheck<0, ImmCheck0_0>, ImmCheck<2, ImmCheck0_7>]>;
}

//
// lookup table expand two contiguous registers
//
let SMETargetGuard = "sme2" in {
  def SVLUTI2_LANE_ZT_X2 : Inst<"svluti2_lane_zt_{d}_x2", "2.di[i", "cUcsUsiUibhf", MergeNone, "aarch64_sme_luti2_lane_zt_x2", [IsStreaming, IsInOutZA], [ImmCheck<0, ImmCheck0_0>, ImmCheck<2, ImmCheck0_7>]>;
  def SVLUTI4_LANE_ZT_X2 : Inst<"svluti4_lane_zt_{d}_x2", "2.di[i", "cUcsUsiUibhf", MergeNone, "aarch64_sme_luti4_lane_zt_x2", [IsStreaming, IsInOutZA], [ImmCheck<0, ImmCheck0_0>, ImmCheck<2, ImmCheck0_3>]>;
// SME2 FP8 instructions
//
}

// FDOT
let SMETargetGuard = "sme-f8f32" in {
  def SVDOT_LANE_FP8_ZA32_VG1x2 : Inst<"svdot_lane_za32[_mf8]_vg1x2", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fdot_lane_za32_vg1x2", [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_3>]>;
  def SVDOT_LANE_FP8_ZA32_VG1x4 : Inst<"svdot_lane_za32[_mf8]_vg1x4", "vm4di>", "m", MergeNone, "aarch64_sme_fp8_fdot_lane_za32_vg1x4", [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_3>]>;

  def SVVDOTB_LANE_FP8_ZA32_VG1x4 : Inst<"svvdotb_lane_za32[_mf8]_vg1x4", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fvdotb_lane_za32_vg1x4", [IsOverloadNone, IsStreaming, IsInOutZA], [ImmCheck<3, ImmCheck0_3>]>;
  def SVVDOTT_LANE_FP8_ZA32_VG1x4 : Inst<"svvdott_lane_za32[_mf8]_vg1x4", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fvdott_lane_za32_vg1x4", [IsOverloadNone, IsStreaming, IsInOutZA], [ImmCheck<3, ImmCheck0_3>]>;

  def SVDOT_SINGLE_FP8_ZA32_VG1x2 : Inst<"svdot[_single]_za32[_mf8]_vg1x2", "vm2d>", "m", MergeNone, "aarch64_sme_fp8_fdot_single_za32_vg1x2", [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVDOT_SINGLE_FP8_ZA32_VG1x4 : Inst<"svdot[_single]_za32[_mf8]_vg1x4", "vm4d>", "m", MergeNone, "aarch64_sme_fp8_fdot_single_za32_vg1x4", [IsStreaming, IsInOutZA, IsOverloadNone], []>;

  def SVDOT_MULTI_FP8_ZA32_VG1x2 : Inst<"svdot_za32[_mf8]_vg1x2", "vm22>", "m", MergeNone, "aarch64_sme_fp8_fdot_multi_za32_vg1x2", [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVDOT_MULTI_FP8_ZA32_VG1x4 : Inst<"svdot_za32[_mf8]_vg1x4", "vm44>", "m", MergeNone, "aarch64_sme_fp8_fdot_multi_za32_vg1x4", [IsStreaming, IsInOutZA, IsOverloadNone], []>;
}

let SMETargetGuard = "sme-f8f16" in {
  def SVDOT_LANE_FP8_ZA16_VG1x2 : Inst<"svdot_lane_za16[_mf8]_vg1x2", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fdot_lane_za16_vg1x2", [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_7>]>;
  def SVDOT_LANE_FP8_ZA16_VG1x4 : Inst<"svdot_lane_za16[_mf8]_vg1x4", "vm4di>", "m", MergeNone, "aarch64_sme_fp8_fdot_lane_za16_vg1x4", [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_7>]>;

  def SVVDOT_LANE_FP8_ZA16_VG1x2 : Inst<"svvdot_lane_za16[_mf8]_vg1x2", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fvdot_lane_za16_vg1x2", [IsOverloadNone, IsStreaming, IsInOutZA], [ImmCheck<3, ImmCheck0_7>]>;

  def SVDOT_SINGLE_FP8_ZA16_VG1x2 : Inst<"svdot[_single]_za16[_mf8]_vg1x2", "vm2d>", "m", MergeNone, "aarch64_sme_fp8_fdot_single_za16_vg1x2", [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVDOT_SINGLE_FP8_ZA16_VG1x4 : Inst<"svdot[_single]_za16[_mf8]_vg1x4", "vm4d>", "m", MergeNone, "aarch64_sme_fp8_fdot_single_za16_vg1x4", [IsStreaming, IsInOutZA, IsOverloadNone], []>;

  def SVDOT_MULTI_FP8_ZA16_VG1x2 : Inst<"svdot_za16[_mf8]_vg1x2", "vm22>", "m", MergeNone, "aarch64_sme_fp8_fdot_multi_za16_vg1x2", [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVDOT_MULTI_FP8_ZA16_VG1x4 : Inst<"svdot_za16[_mf8]_vg1x4", "vm44>", "m", MergeNone, "aarch64_sme_fp8_fdot_multi_za16_vg1x4", [IsStreaming, IsInOutZA, IsOverloadNone], []>;
}

////////////////////////////////////////////////////////////////////////////////
// SME2p1 - FMOPA, FMOPS (non-widening)
let SMETargetGuard = "sme-b16b16" in {
  def SVMOPA_BF16_NW : SInst<"svmopa_za16[_bf16]_m", "viPPdd", "b",
                             MergeNone, "aarch64_sme_mopa",
                             [IsStreaming, IsInOutZA],
                             [ImmCheck<0, ImmCheck0_1>]>;
  def SVMOPS_BF16_NW : SInst<"svmops_za16[_bf16]_m", "viPPdd", "b",
                             MergeNone, "aarch64_sme_mops",
                             [IsStreaming, IsInOutZA],
                             [ImmCheck<0, ImmCheck0_1>]>;
}

let SMETargetGuard = "sme-f16f16" in {
  def SVMOPA_F16_NW : SInst<"svmopa_za16[_f16]_m", "viPPdd", "h",
                            MergeNone, "aarch64_sme_mopa",
                            [IsStreaming, IsInOutZA],
                            [ImmCheck<0, ImmCheck0_1>]>;
  def SVMOPS_F16_NW : SInst<"svmops_za16[_f16]_m", "viPPdd", "h",
                            MergeNone, "aarch64_sme_mops",
                            [IsStreaming, IsInOutZA],
                            [ImmCheck<0, ImmCheck0_1>]>;
}

let SMETargetGuard = "sme-lutv2" in {
  def SVLUTI4_ZT_X4 : SInst<"svluti4_zt_{d}_x4", "4i2.u", "cUc", MergeNone, "aarch64_sme_luti4_zt_x4", [IsStreaming, IsInZT0], [ImmCheck<0, ImmCheck0_0>]>;
}

let SMETargetGuard = "sme-f8f32" in {
  def SVMOPA_FP8_ZA32 : Inst<"svmopa_za32[_mf8]_m", "viPPdd>", "m", MergeNone, "aarch64_sme_fp8_fmopa_za32",
                             [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<0, ImmCheck0_3>]>;
   // FMLALL (indexed)
  def SVMLA_FP8_LANE_ZA32_VG4x1 : Inst<"svmla_lane_za32[_mf8]_vg4x1", "vmddi>", "m", MergeNone, "aarch64_sme_fp8_fmlall_lane_za32_vg4x1",
                                       [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_15>]>;
  def SVMLA_FP8_LANE_ZA32_VG4x2 : Inst<"svmla_lane_za32[_mf8]_vg4x2", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fmlall_lane_za32_vg4x2",
                                       [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_15>]>;
  def SVMLA_FP8_LANE_ZA16_VG4x4 : Inst<"svmla_lane_za32[_mf8]_vg4x4", "vm4di>", "m", MergeNone, "aarch64_sme_fp8_fmlall_lane_za32_vg4x4",
                                       [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_15>]>;
  // FMLALL (single)
  def SVMLA_FP8_SINGLE_ZA32_VG4x1 : Inst<"svmla[_single]_za32[_mf8]_vg4x1", "vmdd>", "m", MergeNone, "aarch64_sme_fp8_fmlall_single_za32_vg4x1",
                                         [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVMLA_FP8_SINGLE_ZA32_VG4x2 : Inst<"svmla[_single]_za32[_mf8]_vg4x2", "vm2d>", "m", MergeNone, "aarch64_sme_fp8_fmlall_single_za32_vg4x2",
                                         [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVMLA_FP8_SINGLE_ZA32_VG4x4 : Inst<"svmla[_single]_za32[_mf8]_vg4x4", "vm4d>", "m", MergeNone, "aarch64_sme_fp8_fmlall_single_za32_vg4x4",
                                         [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  // FMLALL (multiple)
  def SVMLA_FP8_MULTI_ZA32_VG4x2 : Inst<"svmla_za32[_mf8]_vg4x2", "vm22>", "m", MergeNone, "aarch64_sme_fp8_fmlall_multi_za32_vg4x2",
                                        [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVMLA_FP8_MULTI_ZA32_VG4x4 : Inst<"svmla_za32[_mf8]_vg4x4", "vm44>", "m", MergeNone, "aarch64_sme_fp8_fmlall_multi_za32_vg4x4",
                                        [IsStreaming, IsInOutZA, IsOverloadNone], []>;
}

let SMETargetGuard = "sme-f8f16" in {
  def SVMOPA_FP8_ZA16 : Inst<"svmopa_za16[_mf8]_m", "viPPdd>", "m", MergeNone, "aarch64_sme_fp8_fmopa_za16",
                             [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<0, ImmCheck0_1>]>;
  // FMLAL (indexed)
  def SVMLA_FP8_LANE_ZA16_VG2x1 : Inst<"svmla_lane_za16[_mf8]_vg2x1", "vmddi>", "m", MergeNone, "aarch64_sme_fp8_fmlal_lane_za16_vg2x1",
                                       [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_15>]>;
  def SVMLA_FP8_LANE_ZA16_VG2x2 : Inst<"svmla_lane_za16[_mf8]_vg2x2", "vm2di>", "m", MergeNone, "aarch64_sme_fp8_fmlal_lane_za16_vg2x2",
                                       [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_15>]>;
  def SVMLA_FP8_LANE_ZA16_VG2x4 : Inst<"svmla_lane_za16[_mf8]_vg2x4", "vm4di>", "m", MergeNone, "aarch64_sme_fp8_fmlal_lane_za16_vg2x4",
                                       [IsStreaming, IsInOutZA, IsOverloadNone], [ImmCheck<3, ImmCheck0_15>]>;
  // FMLAL (single)
  def SVMLA_FP8_SINGLE_ZA16_VG2x1 : Inst<"svmla[_single]_za16[_mf8]_vg2x1", "vmdd>", "m", MergeNone, "aarch64_sme_fp8_fmlal_single_za16_vg2x1",
                                         [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVMLA_FP8_SINGLE_ZA16_VG2x2 : Inst<"svmla[_single]_za16[_mf8]_vg2x2", "vm2d>", "m", MergeNone, "aarch64_sme_fp8_fmlal_single_za16_vg2x2",
                                         [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVMLA_FP8_SINGLE_ZA16_VG2x4 : Inst<"svmla[_single]_za16[_mf8]_vg2x4", "vm4d>", "m", MergeNone, "aarch64_sme_fp8_fmlal_single_za16_vg2x4",
                                         [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  // FMLAL (multiple)
  def SVMLA_FP8_MULTI_ZA16_VG2x2 : Inst<"svmla_za16[_mf8]_vg2x2", "vm22>", "m", MergeNone, "aarch64_sme_fp8_fmlal_multi_za16_vg2x2",
                                        [IsStreaming, IsInOutZA, IsOverloadNone], []>;
  def SVMLA_FP8_MULTI_ZA16_VG2x4 : Inst<"svmla_za16[_mf8]_vg2x4", "vm44>", "m", MergeNone, "aarch64_sme_fp8_fmlal_multi_za16_vg2x4",
                                        [IsStreaming, IsInOutZA, IsOverloadNone], []>;
}

} // let SVETargetGuard = InvalidMode


