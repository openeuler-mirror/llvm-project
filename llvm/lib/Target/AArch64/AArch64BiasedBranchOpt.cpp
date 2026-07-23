//===-AArch64BiasedBranchOpt.cpp - Optimiza strongly biased branch ---*- C+
//-*-===/
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Copyright (C) 2024, Huawei Technologies Co., Ltd. All rights reserved.
//
//===---------------------------------------------------------------------===//
//
// Read a biased-branch-specific sample profile file and optimize biased branch:
// 1) b.cond => bc.cond
// 2) cbz/cbnz => cmp + bc.cond
//
//===---------------------------------------------------------------------===//

#include "AArch64.h"
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64Subtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/ProfileData/SampleProf.h"
#include "llvm/ProfileData/SampleProfReader.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Transforms/IPO/SampleProfile.h"

#define DEBUG_TYPE "aarch64-biased-branch-opt"

using namespace llvm;
using namespace sampleprof;

static cl::opt<std::string> BiasedBranchInstsProfile(
    "biased-branch-insts-profile", cl::init(""), cl::value_desc("filename"),
    cl::desc("biased branch insts profile to be loaded"), cl::Hidden);

STATISTIC(NumBiasedBranchConverted, "Number of biased branches converted");
STATISTIC(NumBiasedBranchConvertedBCond,
          "Number of biased branches converted that are b.cond");
STATISTIC(NumBiasedBranchConvertedCBZ,
          "Number of biased branches converted that are cbz/cbnz");

namespace {

class AArch64BiasedBranchOpt : public MachineFunctionPass {
public:
  static char ID;
  AArch64BiasedBranchOpt();
  bool doInitialization(Module &) override;
  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override {
    return "AArch64 Biased Branch Optimization";
  }

private:
  std::unique_ptr<SampleProfileReader> Reader;
  bool transformBcond(MachineInstr &MI, MachineBasicBlock &MBB,
                      MachineBasicBlock *TargetMBB, const DebugLoc &DL,
                      const TargetInstrInfo *TII);
  bool transformCBZ(MachineInstr&MI, MachineBasicBlock&MBB,
                    MachineBasicBlock *TargetMBB, const DebugLoc&DL,
                    const TargetInstrInfo*TII);
};
} // end anonymous namespace

char AArch64BiasedBranchOpt::ID = 0;

INITIALIZE_PASS(AArch64BiasedBranchOpt, "aarch64-biased-branch-opt",
                "AArch64 Biased Branch Optimization", false, false)

AArch64BiasedBranchOpt::AArch64BiasedBranchOpt() : MachineFunctionPass(ID) {}

bool AArch64BiasedBranchOpt::doInitialization(Module &M) {
  const std::string &Filename = BiasedBranchInstsProfile;
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

  if (std::error_code EC = Reader->read()) {
    std::string Msg = "profile reading failed: " + EC.message();
    Ctx.diagnose(DiagnosticInfoSampleProfile(Filename, Msg));
    return false;
  }
  return true;
}

bool AArch64BiasedBranchOpt::transformBcond(MachineInstr &MI,
                                            MachineBasicBlock &MBB,
                                            MachineBasicBlock *TargetMBB,
                                            const DebugLoc &DL,
                                            const TargetInstrInfo *TII) {
  assert(MI.getOpcode() == AArch64::Bcc && "opcode should be b.cond!");
  AArch64CC::CondCode CC = (AArch64CC::CondCode)MI.getOperand(0).getImm();
  MachineInstr *NewMI = BuildMI(MBB, MI, DL, TII->get(AArch64::BCcc))
                            .addImm(CC)
                            .addMBB(TargetMBB);
  unsigned ImplicitOpsIdx = 2;
  MachineFunction *MF = MBB.getParent();
  NewMI->removeOperand(ImplicitOpsIdx);
  NewMI->copyImplicitOps(*MF, MI);
  LLVM_DEBUG(dbgs() << ": B.cond -> BC.Cond New MI: ");
  LLVM_DEBUG(NewMI->dump());
  MI.eraseFromParent();
  ++NumBiasedBranchConvertedBCond;
  ++NumBiasedBranchConverted;
  return true;
}

bool AArch64BiasedBranchOpt::transformCBZ(MachineInstr &MI,
                                          MachineBasicBlock &MBB,
                                          MachineBasicBlock *TargetMBB,
                                          const DebugLoc &DL,
                                          const TargetInstrInfo *TII) {
  assert((MI.getOpcode() == AArch64::CBZW || MI.getOpcode() == AArch64::CBZX ||
          MI.getOpcode() == AArch64::CBNZW ||
          MI.getOpcode() == AArch64::CBNZX) &&
          "opcode should be cbz/cbnz!");

  // Make sure current change won't clobber previous NZCV
  // that has been defined but has not been used
  for (MachineInstr &PrevMI : reverse(MBB)) {
    bool FoundNZCVUse = false;
    for (const MachineOperand &MO : PrevMI.implicit_operands()) {
      if (MO.isReg() && MO.getReg() == AArch64::NZCV) {
        if (MO.isDef())
          return false;
        FoundNZCVUse = true;
      }
    }
    if (FoundNZCVUse)
      break;
  }

  Register RegCmp = MI.getOperand(0).getReg();
  unsigned NewRegState = getRegState(MI.getOperand(0));
  bool isGPR64 = AArch64::GPR64RegClass.contains(RegCmp);
  bool isGPR32 = AArch64::GPR32RegClass.contains(RegCmp);
  if (!isGPR32 && !isGPR64)
    return false;

  unsigned CmpOp = isGPR32 ? AArch64::SUBSWri : AArch64::SUBSXri;
  unsigned ZReg = isGPR32 ? AArch64::WZR : AArch64::XZR;
  MachineInstr *CmpInstr = BuildMI(MBB, MI, DL, TII->get(CmpOp), ZReg)
                              .addReg(RegCmp, NewRegState)
                              .addImm(0)
                              .addImm(0);
  AArch64CC::CondCode CC =
      (MI.getOpcode() == AArch64::CBZW || MI.getOpcode() == AArch64::CBZX)
          ? AArch64CC::EQ
          : AArch64CC::NE;
  MachineInstr *NewMI = BuildMI(MBB, MI, DL, TII->get(AArch64::BCcc))
                            .addImm(CC)
                            .addMBB(TargetMBB);

  LLVM_DEBUG(dbgs() << ": CBZ -> CMP New MI: ");
  LLVM_DEBUG(CmpInstr->dump());
  LLVM_DEBUG(dbgs() << ": CBZ -> BC.cond New MI: ");
  LLVM_DEBUG(NewMI->dump());
  MI.eraseFromParent();
  ++NumBiasedBranchConverted;
  ++NumBiasedBranchConvertedCBZ;
  return true;
}

bool AArch64BiasedBranchOpt::runOnMachineFunction(MachineFunction &MF) {
  FunctionSamples *FuncSamples =
      Reader ? Reader->getSamplesFor(MF.getFunction()) : nullptr;
  if (FuncSamples == nullptr || FuncSamples->empty())
    return false;

  auto &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  if (!Subtarget.hasHBC())
    return false;

  bool Changed = false;
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  for (auto &MBB : MF) {
    for (MachineInstr &MI : llvm::make_early_inc_range(MBB.terminators())) {
      const DebugLoc &DL = MI.getDebugLoc();
      const DILocation *DIL = DL.get();
      if (DIL == nullptr)
        continue;

      switch (MI.getOpcode()) {
      case AArch64::Bcc:
      case AArch64::CBZW:
      case AArch64::CBZX:
      case AArch64::CBNZW:
      case AArch64::CBNZX:
        break;
      default:
        continue;
      }

      const FunctionSamples *Samples = FuncSamples->findFunctionSamples(DL);
      if (!Samples)
        continue;

      const ErrorOr<uint64_t> Count = Samples->findSamplesAt(
          FunctionSamples::getOffset(DIL), DIL->getBaseDiscriminator());
      if (!Count)
        continue;

      LLVM_DEBUG(dbgs() << ": Old MI: ");
      LLVM_DEBUG(MI.dump());
      MachineBasicBlock *TargetMBB = TII->getBranchDestBlock(MI);
      if (MI.getOpcode() == AArch64::Bcc)
        Changed |= transformBcond(MI, MBB, TargetMBB, DL, TII);
      else
        Changed |= transformCBZ(MI, MBB, TargetMBB, DL, TII);
    }
  }
  return Changed;
}

FunctionPass *llvm::createAArch64BiasedBranchOptPass() {
  return new AArch64BiasedBranchOpt();
}
