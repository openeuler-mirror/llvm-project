//===------ AArch64InsertPrefetch.cpp - Insert Prefetch instruction ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//
//
// This file implements a AArch64 Insert Prefetch Pass.
// This pass aims to read prefetch hints file and insert prefetch instruction
// for load instruction accroding to hints file.
//
//===---------------------------------------------------------------------===//

#include "AArch64.h"
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64RegisterInfo.h"
#include "AArch64Subtarget.h"
#include "MCTargetDesc/AArch64AddressingModes.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/ProfileData/SampleProf.h"
#include "llvm/ProfileData/SampleProfReader.h"
#include "llvm/Support/VirtualFileSystem.h"

#define DEBUG_TYPE "aarch64-insert-prefetch"

using namespace llvm;
using namespace sampleprof;

cl::opt<bool> EnableAArch64InsertPrefetch(
    "enable-aarch64-insert-prefetch", cl::init(false), cl::ReallyHidden,
    cl::desc("Enable AArch64 Insert Prefetch Pass"));

static cl::opt<std::string> AArch64PrefetchHintsFile(
    "aarch64-prefetch-hints-file",
    cl::desc("Path to the prefetch hints profile"), cl::Hidden,
    cl::callback([](const std::string &Path) {
      if (!Path.empty())
        EnableAArch64InsertPrefetch = true;
    }));

namespace {

class AArch64InsertPrefetch : public MachineFunctionPass {
  bool doInitialization(Module &M) override;
  bool runOnMachineFunction(MachineFunction &MF) override;
  bool processLoad(MachineInstr &MI, MachineBasicBlock &MBB, const DebugLoc &DL,
                   const AArch64InstrInfo *TII, MachineRegisterInfo *MRI,
                   int64_t PrefetchDistance);
  bool insertPrefetchMI(MachineInstr &MI, MachineBasicBlock &MBB,
                        const DebugLoc &DL, const AArch64InstrInfo *TII,
                        MachineRegisterInfo *MRI, int64_t PrefetchDistance,
                        Register BaseReg);

public:
  static char ID;

  explicit AArch64InsertPrefetch(const std::string &PrefetchHintsFilename)
      : MachineFunctionPass(ID), Filename(PrefetchHintsFilename) {}

  StringRef getPassName() const override {
    return "AArch64 Insert Prefetch";
  }

private:
  std::string Filename;
  std::unique_ptr<SampleProfileReader> Reader;
};

char AArch64InsertPrefetch::ID = 0;

bool AArch64InsertPrefetch::doInitialization(Module &M) {
  if (Filename.empty())
    return false;
  LLVMContext &Ctx = M.getContext();
  auto FS = vfs::getRealFileSystem();
  ErrorOr<std::unique_ptr<SampleProfileReader>> ReaderOrErr =
      SampleProfileReader::create(Filename, Ctx, *FS);
  if (std::error_code EC = ReaderOrErr.getError()) {
    std::string Msg = "Could not open profile: " + EC.message();
    Ctx.diagnose(DiagnosticInfoSampleProfile(Filename, Msg,
                                             DiagnosticSeverity::DS_Warning));
    return false;
  }
  Reader = std::move(ReaderOrErr.get());
  Reader->read();
  return true;
}

static AArch64_AM::ShiftExtendType getShiftTypeFromLoad(const MachineInstr &MI) {
  unsigned SignExtend = MI.getOperand(3).getImm();
  switch(MI.getOpcode()) {
    case AArch64::LDRBBroW:
    case AArch64::LDRHHroW:
    case AArch64::LDRWroW:
    case AArch64::LDRXroW:
    case AArch64::LDRQroW:
    case AArch64::LDRSBWroW:
    case AArch64::LDRSBXroW:
    case AArch64::LDRSHWroW:
    case AArch64::LDRSHXroW:
    case AArch64::LDRSWroW:
      return SignExtend ? AArch64_AM::SXTW : AArch64_AM::UXTW;
    case AArch64::LDRBBroX:
    case AArch64::LDRSBWroX:
    case AArch64::LDRSBXroX:
      return SignExtend ? AArch64_AM::SXTX : AArch64_AM::InvalidShiftExtend;
    case AArch64::LDRHHroX:
    case AArch64::LDRWroX:
    case AArch64::LDRXroX:
    case AArch64::LDRQroX:
    case AArch64::LDRSHWroX:
    case AArch64::LDRSHXroX:
    case AArch64::LDRSWroX:
      return SignExtend ? AArch64_AM::SXTX : AArch64_AM::LSL;
    default:
      errs() << "No shift type for " << MI << "\n";
      return AArch64_AM::InvalidShiftExtend;
  }
}

static unsigned getImmOffsetOpcode(const MachineInstr &MI) {
  switch(MI.getOpcode()) {
    case AArch64::LDRBBroW:
    case AArch64::LDRBBroX:
      return AArch64::LDRBBui;
    case AArch64::LDRSBXroW:
    case AArch64::LDRSBXroX:
      return AArch64::LDRSBXui;
    case AArch64::LDRSBWroW:
    case AArch64::LDRSBWroX:
      return AArch64::LDRSBWui;
    case AArch64::LDRHHroW:
    case AArch64::LDRHHroX:
      return AArch64::LDRHHui;
    case AArch64::LDRSHXroW:
    case AArch64::LDRSHXroX:
      return AArch64::LDRSHXui;
    case AArch64::LDRSHWroW:
    case AArch64::LDRSHWroX:
      return AArch64::LDRSHWui;
    case AArch64::LDRWroW:
    case AArch64::LDRWroX:
      return AArch64::LDRWui;
    case AArch64::LDRSWroW:
    case AArch64::LDRSWroX:
      return AArch64::LDRSWui;
    case AArch64::LDRXroW:
    case AArch64::LDRXroX:
      return AArch64::LDRXui;
    case AArch64::LDRQroW:
    case AArch64::LDRQroX:
      return AArch64::LDRQui;
    default:
      errs() << "No immediate offset opcode for " << MI.getOpcode() << "\n";
  }
}

static const MachineOperand getLdDefOp(const MachineInstr &MI) {
  unsigned Opcode = MI.getOpcode();
  unsigned Idx =
      AArch64InstrInfo::isPreLd(MI) || Opcode == AArch64::LDRBpre ||
      Opcode == AArch64::LDRBBpre || Opcode == AArch64::LDRHpre ||
      Opcode == AArch64::LDRHHpre ? 1 : 0;
  return MI.getOperand(Idx);
}

static const MachineOperand getLdBaseOp(const MachineInstr &MI) {
  unsigned Opcode = MI.getOpcode();
  unsigned Idx =
      AArch64InstrInfo::isPreLd(MI) || Opcode == AArch64::LDRBpre ||
      Opcode == AArch64::LDRBBpre || Opcode == AArch64::LDRHpre ||
      Opcode == AArch64::LDRHHpre ? 2 : 1;
  return MI.getOperand(Idx);
}

static const MachineOperand getLdOffsetOp(const MachineInstr &MI) {
  unsigned Opcode = MI.getOpcode();
  unsigned Idx =
      AArch64InstrInfo::isPreLd(MI) || Opcode == AArch64::LDRBpre ||
      Opcode == AArch64::LDRBBpre || Opcode == AArch64::LDRHpre ||
      Opcode == AArch64::LDRHHpre ? 3 : 2;
  return MI.getOperand(Idx);
}

static bool hasUnscaledLdOffset(unsigned Opcode) {
  switch (Opcode) {
  default:
    return false;
  case AArch64::LDURSi:
  case AArch64::LDRSpre:
  case AArch64::LDURDi:
  case AArch64::LDRDpre:
  case AArch64::LDURQi:
  case AArch64::LDRQpre:
  case AArch64::LDURWi:
  case AArch64::LDRWpre:
  case AArch64::LDURXi:
  case AArch64::LDRXpre:
  case AArch64::LDURSWi:
  case AArch64::LDURHi:
  case AArch64::LDRHpre:
  case AArch64::LDURHHi:
  case AArch64::LDRHHpre:
  case AArch64::LDURBi:
  case AArch64::LDRBpre:
  case AArch64::LDURBBi:
  case AArch64::LDRBBpre:
  case AArch64::LDURSBWi:
  case AArch64::LDURSHWi:
    return true;
  }
}

bool AArch64InsertPrefetch::insertPrefetchMI(MachineInstr &MI,
                                             MachineBasicBlock &MBB,
                                             const DebugLoc &DL,
                                             const AArch64InstrInfo *TII,
                                             MachineRegisterInfo *MRI,
                                             int64_t PrefetchDistance,
                                             Register BaseReg) {
  int64_t TotalOffset = PrefetchDistance;
  // If the load instruction uses an immediate offset, add it to the offset of
  // prfm instruction.
  MachineOperand OffsetOp = getLdOffsetOp(MI);
  if (OffsetOp.isImm()) {
    int MemScale = TII->getMemScale(MI);
    int UnscaledLdOffset = hasUnscaledLdOffset(MI.getOpcode())
                              ? OffsetOp.getImm()
                              : OffsetOp.getImm() * MemScale;
    TotalOffset += UnscaledLdOffset;
  }
  // Get the max offset and min offset of PRFUMi,
  TypeSize DummyScale(0U, false);
  unsigned DummyWidth;
  int64_t PRFUMMinOffset, PRFUMMaxOffset;
  AArch64InstrInfo::getMemOpInfo(AArch64::PRFUMi, DummyScale, DummyWidth,
                                 PRFUMMinOffset, PRFUMMaxOffset);
  const int64_t ADDMax = 4095, SUBMax = -4095;

  // Helper function to insert PRFUMi or PRFMui.
  auto InsertPrefetch = [&](unsigned Opcode, Register BaseReg, int64_t Offset) {
      BuildMI(MBB, MI, DL, TII->get(Opcode))
          .addImm(0) // 0 represents pldl1keep.
          .addReg(BaseReg)
          .addImm(Offset);
  };

  // Helper function to insert MOV and PRFMroX.
  auto InsertMovAndPrefetch = [&](Register BaseReg, int64_t Offset) {
    Register MOVDefReg = MRI->createVirtualRegister(&AArch64::GPR64RegClass);
    BuildMI(MBB, MI, DL, TII->get(AArch64::MOVZXi))
        .addDef(MOVDefReg)
        .addImm(Offset)
        .addImm(0);
    BuildMI(MBB, MI, DL, TII->get(AArch64::PRFMroX))
        .addImm(0)
        .addReg(BaseReg)
        .addReg(MOVDefReg)
        .addImm(0)
        .addImm(0);
  };

  // If total offset is within [-256, 255], insert PRFUMi.
  if (TotalOffset <= PRFUMMaxOffset && TotalOffset >= PRFUMMinOffset) {
    InsertPrefetch(AArch64::PRFUMi, BaseReg, TotalOffset);
  } else if (TotalOffset < PRFUMMinOffset && TotalOffset >= SUBMax) {
    // If total offset is within [-4095, -256), insert SUBXri + PRFMui.
    Register SUBDefReg = MRI->createVirtualRegister(&AArch64::GPR64RegClass);
    BuildMI(MBB, MI, DL, TII->get(AArch64::SUBXri))
        .addDef(SUBDefReg)
        .addReg(BaseReg)
        .addImm(-TotalOffset);
    InsertPrefetch(AArch64::PRFMui, SUBDefReg, 0);
  } else if (TotalOffset < SUBMax) {
    // If offset < -4095, insert MOV + PRFMroX.
    InsertMovAndPrefetch(BaseReg, TotalOffset);
  } else { // if (TotalOffset > PRFUMMaxOffset)
    TypeSize PRFMScale(0U, false);
    int64_t PRFMMinOffset, PRFMMaxOffset;
    AArch64InstrInfo::getMemOpInfo(AArch64::PRFMui, PRFMScale, DummyWidth,
                                   PRFMMinOffset, PRFMMaxOffset);
    const unsigned PRFMMax = PRFMMaxOffset * PRFMScale;
    bool IsAligned = (TotalOffset & 7) == 0;
    if (IsAligned && TotalOffset <= PRFMMax) {
      // If offset <= 32760 and is 8 byte aligned, insert PRFMui.
      InsertPrefetch(AArch64::PRFMui, BaseReg, TotalOffset >> 3);
    } else if ((IsAligned && TotalOffset > PRFMMax) ||
               (!IsAligned && TotalOffset > ADDMax)) {
      // Insert MOV + PRFMroX.
      InsertMovAndPrefetch(BaseReg, TotalOffset);
    } else { // if (!IsAligned && TotalOffset <= ADDMax)
      // Insert ADD + PRFMui.
      Register ADDDefReg = MRI->createVirtualRegister(&AArch64::GPR64RegClass);
      BuildMI(MBB, MI, DL, TII->get(AArch64::ADDXri))                            
          .addDef(ADDDefReg)                                                    
          .addReg(BaseReg) 
          .addImm(TotalOffset);
      InsertPrefetch(AArch64::PRFMui, ADDDefReg, 0);
    }
  }
  return true;
}

bool AArch64InsertPrefetch::processLoad(MachineInstr &MI,
                                        MachineBasicBlock &MBB,
                                        const DebugLoc &DL,
                                        const AArch64InstrInfo *TII,
                                        MachineRegisterInfo *MRI,
                                        int64_t PrefetchDistance) {
  // LDP instructions can't be processed yet.
  if (AArch64InstrInfo::isPairedLdSt(MI))
    return false;

  // Extract operands of load instruction.
  MachineOperand DefOp = getLdDefOp(MI);
  MachineOperand BaseOp = getLdBaseOp(MI);
  MachineOperand OffsetOp = getLdOffsetOp(MI);
  if (BaseOp.isFI())
    return false;

  if (!OffsetOp.isReg())
    return insertPrefetchMI(MI, MBB, DL, TII, MRI, PrefetchDistance,
                            BaseOp.getReg());

  // If the load instruction uses register offset, insert add instruction
  // before load and replace it with a new load which uses the result of add
  // as base operand. For example.
  //	ldr w0, [x0, w1, sxtw #2]
  //	=>
  //	add x8, x0, w1, sxtw #2
  //	prfm pldl1keep, [x8, #imm]
  //	ldr w0, [x8]
  unsigned LoadImmOp = getImmOffsetOpcode(MI);
  TypeSize Scale(0U, false);
  unsigned DummyWidth;
  int64_t Dummy1, Dummy2;
  AArch64InstrInfo::getMemOpInfo(LoadImmOp, Scale, DummyWidth, Dummy1, Dummy2);
  unsigned DoShift = MI.getOperand(4).getImm();
  AArch64_AM::ShiftExtendType ShiftType = getShiftTypeFromLoad(MI);
  unsigned ADDOpcode = ShiftType >= AArch64_AM::UXTB
                          ? AArch64::ADDXrx64
                          : AArch64::ADDXrs;
  unsigned ShiftImm = ShiftType >= AArch64_AM::UXTB
                          ? AArch64_AM::getArithExtendImm(ShiftType,
                                                          llvm::Log2_32(Scale))
                          : AArch64_AM::getShifterImm(ShiftType,
                                                      llvm::Log2_32(Scale));
  Register ADDDefReg = MRI->createVirtualRegister(&AArch64::GPR64RegClass);
  // Insert ADD instuction.
  BuildMI(MBB, MI, DL, TII->get(ADDOpcode))
      .addDef(ADDDefReg)
      .addReg(BaseOp.getReg())
      .addReg(OffsetOp.getReg())
      .addImm(DoShift ? ShiftImm : 0);
  // Insert Prefetch instruction.
  insertPrefetchMI(MI, MBB, DL, TII, MRI, PrefetchDistance, ADDDefReg);
  // Replace Load instruction.
  BuildMI(MBB, MI, DL, TII->get(LoadImmOp))
      .addDef(DefOp.getReg())
      .addReg(ADDDefReg)
      .addImm(0)
      .addMemOperand(*MI.memoperands_begin());
  MI.eraseFromParent();
  return true;
}

bool AArch64InsertPrefetch::runOnMachineFunction(MachineFunction &MF) {
  if (!Reader)
    return false;

  const FunctionSamples *Samples = Reader->getSamplesFor(MF.getFunction());
  if (!Samples)
    return false;

  bool Changed = false;
  const AArch64InstrInfo *TII =
      static_cast<const AArch64InstrInfo *>(MF.getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &MF.getRegInfo();

  for (MachineBasicBlock &MBB : MF) {
    for (auto MI = MBB.instr_begin(); MI != MBB.instr_end();) {
      auto Current = MI;
      MI++;
      if (!Current->mayLoad())
        continue;
      const DebugLoc &DL = Current->getDebugLoc();
      const DILocation *DIL = DL.get();
      if (!DIL)
        continue;
      auto *FunctionSamples = Samples->findFunctionSamples(DIL);
      if (!FunctionSamples)
        continue;

      uint32_t Off = FunctionSamples::getOffset(DIL);
      uint32_t Dis = DIL->getBaseDiscriminator();
      auto PrefetchType = FunctionSamples->findCallTargetMapAt(Off, Dis);
      if (!PrefetchType)
        continue;
      for (auto &KV : PrefetchType.get()) {
        StringRef Type = KV.getKey();
        int64_t PrefetchDistance = static_cast<int64_t>(KV.second);
        if (Type == "__load")
          Changed |= processLoad(*Current, MBB, DL, TII, MRI, PrefetchDistance);
        else
          errs() << "Unsupported prefetch type: " << Type << "\n";
      }
    }
  }
  return Changed;
}

} // end anonymous namespace

FunctionPass *llvm::createAArch64InsertPrefetchPass() {
  return new AArch64InsertPrefetch(AArch64PrefetchHintsFile);
}

