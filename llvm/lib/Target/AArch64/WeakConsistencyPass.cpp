//===- WeakConsistencyPass.cpp - Weak Consistency Pass ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//This pass is a weak memory sequence compilation repair tool.
//Basic repair logic: During compilation, a barrier (dmb sy) is automatically 
//inserted before the ldr/str instruction, or ldar/stlr is automatically replaced.
//
//===----------------------------------------------------------------------===//
 
#include "AArch64.h"
#include "WeakConsistencyAllowlist.h"
#include "WeakConsistencyConfig.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Module.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
 


namespace {
const std::string WEAKCONSISTENCYPASS_NAME = "WeakConsistencyPass";
const int WEAKCONSISTENCY_LARGE_FUNCTION = 9000;
}
 
using llvm::MachineInstr;
using llvm::MachineFunction;
using llvm::GlobalValue;
using llvm::MachineMemOperand;
using llvm::isa;
using llvm::cast;
using llvm::MachineBasicBlock;
 
enum ModifyType : uint8_t {
    NO_MODIFY,
    ADD_DMB,
    DELETE_MI,
};
 
namespace opts {
static llvm::cl::opt<RELAXED_ORDERING_LEVEL>
    level("relaxed-ordering-level", llvm::cl::desc("Set relaxed level"),
          llvm::cl::init(RO_DISABLE),
          llvm::cl::values(clEnumValN(RO_DISABLE, "disable",



                                      "disable weakconsistency pass"),
                           clEnumValN(RO_1, "0", "level 0"),
                           clEnumValN(RO_2, "1", "level 1"),
                           clEnumValN(RO_3, "2", "level 2")),
          llvm::cl::ZeroOrMore, llvm::cl::NotHidden);
 
static llvm::cl::opt<std::string>
    allowlist("relaxed-ordering-allowlist",
              llvm::cl::desc("Set relaxed ordering allow list"),
              llvm::cl::NotHidden);
} // end namespace opts
 
namespace {
class BaseWeakConsistencyPass : public llvm::MachineFunctionPass {
public:
    static char ID;
    BaseWeakConsistencyPass() : llvm::MachineFunctionPass(ID) {}
    virtual bool runOnMachineFunction(MachineFunction &MF) override;
    llvm::StringRef getPassName() const override final {
        return WEAKCONSISTENCYPASS_NAME;
    }
};
 
class WeakConsistencyPass_level1 : public BaseWeakConsistencyPass {
public:
    WeakConsistencyPass_level1();
    bool runOnMachineFunction(MachineFunction &MF) final;
 
protected:
    virtual ModifyType checkMI(MachineInstr &MI) const;
    virtual void preCheckMI(MachineInstr &MI);
    virtual void postCheckMI(MachineInstr &MI);
    virtual bool initPass(const MachineFunction &MF);
    bool isFrameSetupOrDestroy(const MachineInstr &MI) const;
 
    bool hasDMB = false;
    WeakConsistencyAllowlist allowlist;
 
private:
    bool isVirtualTable(const MachineInstr &MI) const;
    bool isThreadLocal(const MachineInstr &MI) const;
    bool isFPOrSPOperand(const MachineInstr &MI) const;
    virtual bool checkOpcode(const MachineInstr &MI) const;
 
    int totalInst = 0;
    int ldstInst = 0;
};
 
class WeakConsistencyPass_level2 : public WeakConsistencyPass_level1 {
public:
    WeakConsistencyPass_level2() = default;
 
protected:
    void preCheckMI(MachineInstr &MI) override;
    void postCheckMI(MachineInstr &MI) override;
    ModifyType checkMI(MachineInstr &MI) const override;
    bool initPass(const MachineFunction &MF) override;
    bool isNormalRegs(const MachineInstr &MI) const;
    bool isNormalLoad(const MachineInstr &MI) const;
    bool isNormalStore(const MachineInstr &MI) const;
 
    std::unordered_set<unsigned int> localRegs;
    std::vector<MachineInstr *> toRemove;
 
private:
    bool isGotOperand(const MachineInstr &MI) const;
    void updateLocalRegs(const MachineInstr &MI);
    bool isLocalRegOpt(const MachineInstr &MI) const;
    bool isAtomic(const MachineInstr &MI) const;
    void removeLocalRegs(const std::vector<unsigned int> &regs);
    void removeLocalReg(unsigned int reg);
    bool isAsCheapAsMove(const MachineInstr &MI) const;
};
 
class WeakConsistencyPass_level3 : public WeakConsistencyPass_level2 {
public:
    WeakConsistencyPass_level3() = default;
 
protected:
    ModifyType checkMI(MachineInstr &MI) const override;
    bool initPass(const MachineFunction &MF) override;
 
private:
    void useLdar(MachineInstr &MI, unsigned int opcode) const;
    void useLdaxr(MachineInstr &MI) const;
    bool tryToUseLdar(MachineInstr &MI) const;
    bool checkLdstInst(MachineInstr &MI, unsigned min_align, unsigned opts, unsigned opcode) const;
    bool checkRegists(const MachineInstr &MI) const;
    void addLdarImm(MachineInstr &MI, int64_t imm, bool isPost) const;
    bool isPostLdstInst(const MachineInstr &MI) const;
    unsigned getTargetLdpCode(const MachineInstr &MI) const;
    bool isAligned(const MachineMemOperand &MI, unsigned min_align) const;
    void useCASAL(MachineInstr &MI) const;
    bool checkOpcode(const MachineInstr &MI) const override;
};
 
} // end anonymous namespace
 
llvm::FunctionPass *llvm::createWeakConsistencyPass() {
    switch (opts::level) {
    case RELAXED_ORDERING_LEVEL::RO_DISABLE:
        return new BaseWeakConsistencyPass();
    case RELAXED_ORDERING_LEVEL::RO_1:
        return new WeakConsistencyPass_level1();
    case RELAXED_ORDERING_LEVEL::RO_2:
        return new WeakConsistencyPass_level2();
    case RELAXED_ORDERING_LEVEL::RO_3:
        return new WeakConsistencyPass_level3();
    }
    return new BaseWeakConsistencyPass();
}
 
bool WeakConsistencyPass_level1::initPass(const MachineFunction &MF) {
    totalInst = 0;
    ldstInst = 0;
    for (auto &MBB : MF) {
        for (auto &MI : MBB) {
            totalInst++;
            if (MI.mayLoadOrStore()) 
                ldstInst ++;
        }
    }
    if (totalInst > WEAKCONSISTENCY_LARGE_FUNCTION) {
        llvm::WithColor::error(llvm::errs(), "WeakConsistencyPass")
            << "Ignore large funtion: " << MF.getName() << "\n";
        return false;
    }
 
    auto &filename = MF.getFunction().getParent()->getSourceFileName();
    return allowlist.Check(filename, MF.getName().str());
}
 
bool WeakConsistencyPass_level2::initPass(const MachineFunction &MF) {
    if (!WeakConsistencyPass_level1::initPass(MF))
        return false;
    
    localRegs = {llvm::AArch64::SP, llvm::AArch64::FP, llvm::AArch64::LR};
    
     // X0 of the constructor is the memory address newly allocated to the current 
     // thread and has not been synchronized to other threads. In this case, X0 can be 
     // securely identified as a local variable.
    llvm::ItaniumPartialDemangler IPD;
    if (!IPD.partialDemangle(MF.getName().data())) 
        if (IPD.isCtorOrDtor()) 
            localRegs.insert(llvm::AArch64::X0);
    return true;
}
 
bool WeakConsistencyPass_level3::initPass(const MachineFunction &MF) {
    if (!WeakConsistencyPass_level2::initPass(MF))
        return false;
    
     // Do not process the first four parameters (load/store) transferred by the function.
     //   ldr	x19, [x0, #8]    // The first-layer LDR operation of the input parameter is not processed.
     //   cbz	x19, .LBB4_16
     //   dmb	sy
     //   ldr	x0, [x19, #4912] // Add dmb to the second input ldr parameter. 
    for (int i = llvm::AArch64::X0; i <= llvm::AArch64::X3; i++) 
        localRegs.insert(i);
    return true;
}
 
char BaseWeakConsistencyPass::ID = 0;
bool BaseWeakConsistencyPass::runOnMachineFunction(MachineFunction &MF) {
    return false;
}
 
WeakConsistencyPass_level1::WeakConsistencyPass_level1()
    : BaseWeakConsistencyPass()
{
    if (!opts::allowlist.empty()) 
        allowlist.Initialize(opts::allowlist);
}
 
bool WeakConsistencyPass_level1::runOnMachineFunction(MachineFunction &MF) {
    if (!initPass(MF)) 
        return false;
 
    bool modified = false;
    const llvm::TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
    for (auto &MBB : MF) {
        MachineBasicBlock::iterator E = MBB.end();
        MachineBasicBlock::iterator  NextI;
        for (auto MII = next_nodbg(MBB.begin(), E); MII != E; MII = NextI) {
            NextI = next_nodbg(MII, E);
            preCheckMI(*MII);
            ModifyType ret = checkMI(*MII);
            postCheckMI(*MII);
            if (ret == ADD_DMB) {
                BuildMI(MBB, *MII, MII->getDebugLoc(), TII.get(llvm::AArch64::DMB)).addImm(0xf);
                modified = true;
            }
            if (ret == DELETE_MI) 
                modified = true;
        }
    }
 
    return modified;
}
 
namespace {
static inline const llvm::StringRef getCallFunctionName(const MachineInstr &MI) {
    if (!MI.isCall()) 
        return "";
    
    const auto &op = MI.getOperand(0);
    switch (op.getType()) {
    case llvm::MachineOperand::MO_GlobalAddress:{
        return op.getGlobal()->getName();
    }
    default:
        return "";
    }
}
 
static std::vector<unsigned int> _getRegs(const MachineInstr &MI, bool ignoreZR = true) {
    std::vector<unsigned int> regs;
    for (auto operand : MI.operands()) {
        if (operand.isReg() && !operand.isImplicit() && !operand.isEarlyClobber()) {
            auto id = operand.getReg().id();
            if (!ignoreZR || (id != llvm::AArch64::XZR && id != llvm::AArch64::WZR)) 
                regs.push_back(operand.getReg().id());
        }
    }
    return regs;
}
 
bool isDmbInst(const MachineInstr &MI) {
    static const std::unordered_set<unsigned int> idmbs = {
        llvm::AArch64::DMB,
        llvm::AArch64::DSB,
        llvm::AArch64::ISB,
        llvm::AArch64::DSBnXS
    };
    auto opcode = MI.getOpcode();
    return idmbs.find(opcode) != idmbs.end();
}
 
inline bool isInRange(unsigned opcode, unsigned start, unsigned end) {
    return opcode >= start && opcode <= end;
}
 
// Find the call malloc... instructions
inline bool isNewMalloc(const MachineInstr &MI) {
    static const std::unordered_set<std::string> news = {
        "malloc", "calloc", "_Znwm", "realloc"
    };
    llvm::StringRef name = getCallFunctionName(MI);
    return news.find(name.str()) != news.end();
}
} // end anonymous namespace
 
// Instructions such as clrex hint are not fixed.
bool WeakConsistencyPass_level1::checkOpcode(const MachineInstr &MI) const {
    auto code = MI.getOpcode();
    return code == llvm::AArch64::CLREX || code == llvm::AArch64::HINT;
}
 
// [c,c++] Check whether the function is initialized or part of the memory is returned.
// initialize:  frame-setup STPXi killed $fp, killed $lr, $sp, 2
// return: $fp, $lr = frame-destroy LDPXi $sp, 2
bool WeakConsistencyPass_level1::isFrameSetupOrDestroy(const MachineInstr &MI) const {
    return MI.getFlag(MachineInstr::FrameSetup) || MI.getFlag(MachineInstr::FrameDestroy);
}
 
// [c,c++] Check whether the address is read or written in the SP/FP.
bool WeakConsistencyPass_level1::isFPOrSPOperand(const MachineInstr &MI) const {
    std::vector<unsigned int> regs(_getRegs(MI));
    for (auto id : regs) 
        if (id == llvm::AArch64::SP || id == llvm::AArch64::FP || id == llvm::AArch64::LR)
            return true;
    return false;
}
 
// [c,c++] Check the thread_local variable operation.
bool WeakConsistencyPass_level1::isThreadLocal(const MachineInstr &MI) const {
    for (auto oprand = MI.memoperands_begin(); oprand != MI.memoperands_end(); oprand ++) {
        const llvm::MachineMemOperand *MMO = *oprand;
        if (MMO->getValue() == nullptr)
            continue;
        
        const llvm::Value &val = *MMO->getValue();
        if (isa<GlobalValue>(val) && cast<const GlobalValue>(val).isThreadLocal())
            return true;
    }
    return false;
}
 
// [c++] Check whether the operation is a virtual table read operation.
// LDRXui killed $x9, target-flags(aarch64-pageoff, aarch64-got, aarch64-nc) @_ZTVN4llvm16itanium_
bool WeakConsistencyPass_level1::isVirtualTable(const MachineInstr &MI) const {
    if (!MI.mayLoad() || MI.getNumMemOperands() > 0)
        return false;
    
    for (auto operand = MI.operands_begin(); operand != MI.operands_end(); operand++) 
        if (operand->getType() == llvm::MachineOperand::MO_GlobalAddress)
            return operand->getGlobal()->getName().startswith("_ZTVN");
    return false;
}
 
void WeakConsistencyPass_level1::preCheckMI(MachineInstr &MI) {
    if (isDmbInst(MI)) {
        hasDMB = true;
        return;
    }
}
 
void WeakConsistencyPass_level1::postCheckMI(MachineInstr &MI) {
    if (hasDMB)
        if (MI.mayLoadOrStore()) 
            hasDMB = false;
}
 
 
// LEVEL 1: Do not add memory barriers before local variables
// If true is returned, the memory barrier needs to be inserted. If false is returned, the 
// memory barrier does not need to be inserted.
ModifyType WeakConsistencyPass_level1::checkMI(MachineInstr &MI) const {
    // dmb The mayLoadOrStore instruction returns true.
    if (!MI.mayLoadOrStore() || isDmbInst(MI)) 
        return NO_MODIFY;
    
    if (isFrameSetupOrDestroy(MI)) 
        return NO_MODIFY;
    
    if (checkOpcode(MI)) 
        return NO_MODIFY;
    
    if (hasDMB) 
        return NO_MODIFY;
    
    if (isFPOrSPOperand(MI)) 
        return NO_MODIFY;
    
    if (isThreadLocal(MI)) 
        return NO_MODIFY;
    
    if (isVirtualTable(MI)) 
        return NO_MODIFY;
    
    return ADD_DMB;
}
 
// [c,c++] Indicates whether the operation is a Got operation.
// LDRXui killed $x0, target-flags(aarch64-pageoff, aarch64-got, aarch64-nc) @_ZSt4cout
bool WeakConsistencyPass_level2::isGotOperand(const MachineInstr &MI) const {
    if (MI.getNumMemOperands() > 0 || !MI.mayLoad()) 
        return false;
 
    for (unsigned i = 1; i < MI.getNumOperands(); i++) {
        auto &operand = MI.getOperand(i);
        if (!operand.isGlobal()) 
            continue;
        
        if (operand.getTargetFlags() & llvm::AArch64II::MO_GOT) 
            return true;
    }
    return false;
}
 
void WeakConsistencyPass_level2::removeLocalReg(unsigned int reg) {
    // Convert w0 to x0
    if (isInRange(reg, llvm::AArch64::W0, llvm::AArch64::W28)) 
        reg += llvm::AArch64::X0 - llvm::AArch64::W0;
    
    if (isInRange(reg, llvm::AArch64::X0, llvm::AArch64::X28)) 
        localRegs.erase(reg);
}
 
void WeakConsistencyPass_level2::removeLocalRegs(const std::vector<unsigned int> &regs) {
    for (auto id : regs) 
        removeLocalReg(id);
}
 
// Check whether the current load and store addresses are unique to threads based on the context.
// example: constructor_and_new.s
bool WeakConsistencyPass_level2::isLocalRegOpt(const MachineInstr &MI) const {
    std::vector<unsigned int> regs(_getRegs(MI, false));
    if (regs.size() < 2) 
        return false;
    
    if (isNormalLoad(MI) || isNormalStore(MI)) 
        return localRegs.find(regs[1]) != localRegs.end();
    
    return localRegs.find(regs.back()) != localRegs.end();
}
 
// [c,c++] Radical policy: Do not process read/write instructions that contain atomic operations.
bool WeakConsistencyPass_level2::isAtomic(const MachineInstr &MI) const {
    auto opcode = MI.getOpcode();
    if (isInRange(opcode, llvm::AArch64::LDARB, llvm::AArch64::LDARX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::LDAPRB, llvm::AArch64::LDAXRX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::STLRB, llvm::AArch64::STLRX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::STLLRB, llvm::AArch64::STLXRX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::LDEORAB, llvm::AArch64::LDEORX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::LDADDAB, llvm::AArch64::LDADDX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::CASALB, llvm::AArch64::CASALX)) 
        return true;
    
    if (isInRange(opcode, llvm::AArch64::CASPALW, llvm::AArch64::CASPALX)) 
        return true;
    
    return false;
}
 
bool WeakConsistencyPass_level2::isAsCheapAsMove(const MachineInstr &MI) const {
    static const std::unordered_set<unsigned> opcodes = {
        llvm::AArch64::ORRXrs, llvm::AArch64::ORRXri,
        llvm::AArch64::EXTRWrri, llvm::AArch64::EXTRXrri,
        llvm::AArch64::UBFMWri, llvm::AArch64::UBFMXri,
    };
    auto code = MI.getOpcode();
    if (isInRange(code, llvm::AArch64::ADDXri, llvm::AArch64::ADDXrx)) 
        return true;
    
    if (MI.isAsCheapAsAMove()) 
        return true;
    
    return opcodes.find(code) != opcodes.end();
}
 
void WeakConsistencyPass_level2::preCheckMI(MachineInstr &MI) {
    WeakConsistencyPass_level1::preCheckMI(MI);
    auto code = MI.getOpcode();
    if (isFrameSetupOrDestroy(MI) || isDmbInst(MI) || MI.isCompare() || MI.mayStore()) 
        return;
    
    if (isNewMalloc(MI)) {
        localRegs.insert(llvm::AArch64::X0);
        return;
    }
    if (MI.mayLoad() || MI.isConditionalBranch() || code == llvm::AArch64::KILL) 
        return;
    
    if (MI.isCall()) {
        localRegs.erase(llvm::AArch64::X0);
        return;
    }
    std::vector<unsigned int> regs(_getRegs(MI, false));
    if (regs.empty()) 
        return;
    
    if (isAsCheapAsMove(MI)) {
        bool found = false;
        for (unsigned i = 1; i < regs.size(); i++) {
            if (localRegs.find(regs[i]) != localRegs.end()) {
                found = true;
                break;
            }
        }
        if (found) 
            localRegs.insert(regs[0]);
        else 
            removeLocalReg(regs[0]);
        return;
    }
    if (!isInRange(code, llvm::AArch64::ADR, llvm::AArch64::ADR_UXTW_ZZZ_D_3)) 
        regs.pop_back();
    
    removeLocalRegs(regs);
}
 
bool WeakConsistencyPass_level2::isNormalRegs(const MachineInstr &MI) const {
    auto regs = _getRegs(MI);
    for (auto reg : regs) {
        if (isInRange(reg, llvm::AArch64::X22_X23_X24_X25_X26_X27_X28_FP, llvm::AArch64::X26_X27)) 
            continue;
        
        if (isInRange(reg, llvm::AArch64::W0, llvm::AArch64::X28)) 
            continue;
        
        static const std::unordered_set<unsigned> ots = {
            llvm::AArch64::SP, llvm::AArch64::FP, llvm::AArch64::LR, llvm::AArch64::WZR, llvm::AArch64::XZR
        };
        if (ots.find(reg) == ots.end()) 
            return false; 
    }
    return true;
}
 
bool WeakConsistencyPass_level2::isNormalLoad(const MachineInstr &MI) const
{
    auto code = MI.getOpcode();
    if (isInRange(code, llvm::AArch64::LDRBBpost, llvm::AArch64::LDRBui)) 
        return true;
    
    if (isInRange(code, llvm::AArch64::LDRHHpost, llvm::AArch64::LDRHui)) 
        return true;
    
    if (isInRange(code, llvm::AArch64::LDRWl, llvm::AArch64::LDRWui)) 
        return true;
    
    if (isInRange(code, llvm::AArch64::LDRXl, llvm::AArch64::LDRXui)) 
        return true;
    
    return false;
}
 
bool WeakConsistencyPass_level2::isNormalStore(const MachineInstr &MI) const
{
    auto code = MI.getOpcode();
    if (isInRange(code, llvm::AArch64::STRBBpost, llvm::AArch64::STRBui)) 
        return true;
    
    if (isInRange(code, llvm::AArch64::STRHHpost, llvm::AArch64::STRHui)) 
        return true;
    
    if (isInRange(code, llvm::AArch64::STRWpost, llvm::AArch64::STRWui)) 
        return true;
    
    if (isInRange(code, llvm::AArch64::STRXpost, llvm::AArch64::STRXui)) 
        return true;
    
    return false;
}
 
void WeakConsistencyPass_level2::postCheckMI(MachineInstr &MI) {
    WeakConsistencyPass_level1::postCheckMI(MI);
    std::vector<unsigned int> regs(_getRegs(MI, false));
    if (MI.mayLoad() && !regs.empty() && isNormalRegs(MI)) {
        removeLocalReg(regs[0]);
        if (!isNormalLoad(MI)) 
            removeLocalReg(regs[1]);
    }
}
 
// LEVEL 2: No memory barrier is added to the aliases of local variables.
// If true is returned, the memory barrier needs to be inserted. If false is returned, 
// the memory barrier does not need to be inserted.
ModifyType WeakConsistencyPass_level2::checkMI(MachineInstr &MI) const {
    ModifyType ret = WeakConsistencyPass_level1::checkMI(MI);
    if (ret != ADD_DMB) 
        return ret;
    
    if (isGotOperand(MI)) 
        return NO_MODIFY;
    
    if (isLocalRegOpt(MI)) 
        return NO_MODIFY;
    
    if (isAtomic(MI)) 
        return NO_MODIFY;
    
    return ADD_DMB;
}
 
void WeakConsistencyPass_level3::useLdar(MachineInstr &MI, unsigned int opcode) const {
    const llvm::TargetInstrInfo &TII = *MI.getMF()->getSubtarget().getInstrInfo();
 
    MI.setDesc(TII.get(opcode));
    for (int i = MI.getNumOperands() - 1; i >= 0; i--) 
        if (!MI.getOperand(i).isReg()) 
            MI.removeOperand(i);    
}
 
// Convert:
//   ldr    x0, [x1], imm
// to:
//   ldar   x0, [x1]
//   add    x1, x1, imm
// Convert:
//   ldr    x0, [x1, imm]
// to:
//   add    x1, x1, imm
//   ldar   x0, [x1]
void WeakConsistencyPass_level3::addLdarImm(MachineInstr &MI, int64_t imm, bool isPrePost) const {
    llvm::MachineBasicBlock *const MBB = MI.getParent();
    if (!isPrePost || imm == 0) 
        return;
    
    auto &operand = MI.getOperand(0);
    for (int i = MI.getNumOperands() - 1; i >= 0; i--) {
        operand = MI.getOperand(i);
        if (operand.isReg()) 
            break;
    }
 
    const llvm::TargetInstrInfo &TII = *MI.getMF()->getSubtarget().getInstrInfo();
    if (imm > 0) 
        BuildMI(*MBB, MI, MI.getDebugLoc(), TII.get(llvm::AArch64::ADDXrx))
            .add(operand)
            .add(operand)
            .addImm(imm);
    else {
        imm = -imm;
        BuildMI(*MBB, MI, MI.getDebugLoc(), TII.get(llvm::AArch64::SUBXrx))
            .add(operand)
            .add(operand)
            .addImm(imm);
    }
}
 
bool WeakConsistencyPass_level3::isPostLdstInst(const MachineInstr &MI) const {
    static const std::unordered_set<unsigned> postInsts = {
        llvm::AArch64::LDRBBpre,
        llvm::AArch64::LDRBpre,
        llvm::AArch64::LDRHHpre,
        llvm::AArch64::LDRHpre,
        llvm::AArch64::LDRWpre,
        llvm::AArch64::LDRXpre,
        llvm::AArch64::STRBBpre,
        llvm::AArch64::STRBpre,
        llvm::AArch64::STRHHpre,
        llvm::AArch64::STRHpre,
        llvm::AArch64::STRWpre,
        llvm::AArch64::STRXpre,
        llvm::AArch64::LDPXpre,
        llvm::AArch64::STPXpre,
    };
    return postInsts.find(MI.getOpcode()) != postInsts.end();
}
 
bool WeakConsistencyPass_level3::isAligned(const MachineMemOperand &MI, unsigned min_align) const {
    unsigned align = MI.getAlign().value();
    unsigned baseAlign = MI.getBaseAlign().value();
 
    return align != 0 && baseAlign != 0 && align % min_align == 0 && align % min_align == 0;
}
 
// In the target load/store instruction, if [xn] and imm meet the alignment requirements, 
// replace ldar/stlr with ldar/stlr.
bool WeakConsistencyPass_level3::checkLdstInst(
    MachineInstr &MI, unsigned min_align, unsigned opts, unsigned distCode) const
{
    if (MI.getNumMemOperands() == 0) 
        return false;
    
    auto &memOperand = *(MI.memoperands_end() - 1);
    if (!isAligned(*memOperand, min_align)) 
        return false;
    
    if (MI.getNumOperands() != opts) 
        return false;
    
    auto &operand = *(MI.operands_end() - 1);
    if (!operand.isImm()) 
        return false;
    
    auto imm = operand.getImm();
    // imm != 0 need to call addLdarImm once. Currently, this function is faulty and will be fixed later
    if (imm != 0) 
        return false;
    
    addLdarImm(MI, imm, !isPostLdstInst(MI));
    useLdar(MI, distCode);
    addLdarImm(MI, imm, isPostLdstInst(MI));
    return true;
}
 
void WeakConsistencyPass_level3::useLdaxr(MachineInstr &MI) const {
    static const std::unordered_map<unsigned, unsigned> opts = {
        {llvm::AArch64::LDXPW, llvm::AArch64::LDAXPW},
        {llvm::AArch64::LDXPX, llvm::AArch64::LDAXPX},
        {llvm::AArch64::LDXRB, llvm::AArch64::LDAXRB},
        {llvm::AArch64::LDXRH, llvm::AArch64::LDAXRH},
        {llvm::AArch64::LDXRW, llvm::AArch64::LDAXRW},
        {llvm::AArch64::LDXRX, llvm::AArch64::LDAXRX},
 
        {llvm::AArch64::STXPW, llvm::AArch64::STLXPW},
        {llvm::AArch64::STXPX, llvm::AArch64::STLXPX},
        {llvm::AArch64::STXRB, llvm::AArch64::STLXRB},
        {llvm::AArch64::STXRH, llvm::AArch64::STLXRH},
        {llvm::AArch64::STXRW, llvm::AArch64::STLXRW},
        {llvm::AArch64::STXRX, llvm::AArch64::STLXRX},
    };
 
    const llvm::TargetInstrInfo &TII = *MI.getMF()->getSubtarget().getInstrInfo();
    auto it = opts.find(MI.getOpcode());
    if (it != opts.end()) 
        MI.setDesc(TII.get(it->second));
}
 
 
void WeakConsistencyPass_level3::useCASAL(MachineInstr &MI) const {
    static const std::unordered_set<unsigned> opB = {
        llvm::AArch64::CASAB, llvm::AArch64::CASLB, llvm::AArch64::CASB
    };
    static const std::unordered_set<unsigned> opH = {
        llvm::AArch64::CASAH, llvm::AArch64::CASLH, llvm::AArch64::CASH
    };
    static const std::unordered_set<unsigned> opW = {
        llvm::AArch64::CASAW, llvm::AArch64::CASLW, llvm::AArch64::CASW
    };
    static const std::unordered_set<unsigned> opX = {
        llvm::AArch64::CASAX, llvm::AArch64::CASLX, llvm::AArch64::CASX
    };
    static const std::unordered_set<unsigned> opPW = {
        llvm::AArch64::CASPAW, llvm::AArch64::CASPLW, llvm::AArch64::CASPW
    };
    static const std::unordered_set<unsigned> opPX = {
        llvm::AArch64::CASPAX, llvm::AArch64::CASPLX, llvm::AArch64::CASPX
    };
    const llvm::TargetInstrInfo &TII = *MI.getMF()->getSubtarget().getInstrInfo();
    auto code = MI.getOpcode();
    unsigned distCode = 0;
    if (opB.find(code) != opB.end()) 
        distCode = llvm::AArch64::CASALB;
    else if (opH.find(code) != opH.end()) 
        distCode = llvm::AArch64::CASALH;
    else if (opW.find(code) != opW.end()) 
        distCode = llvm::AArch64::CASALW;
    else if (opX.find(code) != opX.end()) 
        distCode = llvm::AArch64::CASALX;
    else if (opPW.find(code) != opPW.end())
        distCode = llvm::AArch64::CASPALW;
    else if (opPX.find(code) != opPX.end()) 
        distCode = llvm::AArch64::CASPALX;
    if (distCode != 0) 
        MI.setDesc(TII.get(distCode));
}
 
// [c,c++] Convert ldr to ldar as much as possible.
bool WeakConsistencyPass_level3::tryToUseLdar(MachineInstr &MI) const {
    auto code = MI.getOpcode();
    if (isInRange(code, llvm::AArch64::LDXPW, llvm::AArch64::LDXRX)
        || isInRange(code, llvm::AArch64::STXPW, llvm::AArch64::STXRX)) {
        useLdaxr(MI);
        return true;
    }
    if (isInRange(code, llvm::AArch64::CASAB, llvm::AArch64::CASX)) {
        useCASAL(MI);
        return true;
    }
    if (isInRange(code, llvm::AArch64::LDRBBpost, llvm::AArch64::LDRBui)
         || isInRange(code, llvm::AArch64::STRBBpost, llvm::AArch64::STRBui)) 
        return checkLdstInst(MI, 1, 3, MI.mayLoad() ? llvm::AArch64::LDARB : llvm::AArch64::STLRB);
    
    if (isInRange(code, llvm::AArch64::LDRHHpost, llvm::AArch64::LDRHui)
         || isInRange(code, llvm::AArch64::STRHHpost, llvm::AArch64::STRHui)) 
        return checkLdstInst(MI, 2, 3, MI.mayLoad() ? llvm::AArch64::LDARH : llvm::AArch64::STLRH);
    
    if (isInRange(code, llvm::AArch64::LDRWpost, llvm::AArch64::LDRWui)
         || isInRange(code, llvm::AArch64::STRWpost, llvm::AArch64::STRWui)) 
        return checkLdstInst(MI, 4, 3, MI.mayLoad() ? llvm::AArch64::LDARW : llvm::AArch64::STLRW);
    
    if (isInRange(code, llvm::AArch64::LDRXpost, llvm::AArch64::LDRXui)
         || isInRange(code, llvm::AArch64::STRXpost, llvm::AArch64::STRXui)) 
        return checkLdstInst(MI, 8, 3, MI.mayLoad() ? llvm::AArch64::LDARX : llvm::AArch64::STLRX);
    
    return false;
}
 
bool WeakConsistencyPass_level3::checkOpcode(const MachineInstr &MI) const {
    auto code = MI.getOpcode();
    if (isInRange(code, llvm::AArch64::CPYE, llvm::AArch64::CPY_ZPzI_S))
        return true;
    
    if (MI.isInlineAsm())
        return true;
    
    return isInRange(code, llvm::AArch64::MOPSMemoryCopyPseudo, llvm::AArch64::MOPSMemorySetTaggingPseudo);
}
 
// Only the read and write instructions of the W0-Z28 register are repaired.
bool WeakConsistencyPass_level3::checkRegists(const MachineInstr &MI) const {
    return !isNormalRegs(MI);
}
 
// LEVEL 3: Use aggressive algorithms to further reduce memory barrier insertions
// If true is returned, the memory barrier needs to be inserted. If false is returned, 
// the memory barrier does not need to be inserted.
ModifyType WeakConsistencyPass_level3::checkMI(MachineInstr &MI) const {
    ModifyType ret = WeakConsistencyPass_level2::checkMI(MI);
    if (ret != ADD_DMB) 
        return ret;
    
    if (checkOpcode(MI)) 
        return NO_MODIFY;
    
    if (checkRegists(MI)) 
        return NO_MODIFY;
    
    if (tryToUseLdar(MI)) 
        return DELETE_MI;
    
    return ADD_DMB;
}

