#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/Local.h"

#define DEBUG_TYPE "remove-dead-code"

using namespace llvm;

namespace {
    // Indicates whether the current optimization iteration modified the IR.
    static bool Changed;

    // Remove instructions that LLVM identifies as trivially dead.
    static void eliminateDeadInstructions(Function &F) {
        for (BasicBlock &BB : reverse(F)) {
            // Early-increment iteration allows instructions to be erased safely.
            for (Instruction &I : make_early_inc_range(reverse(BB))) {
                if (isInstructionTriviallyDead(&I)) {
                    I.eraseFromParent();
                    Changed = true;
                }
            }
        }
    }

    // Simplify redundant branches and remove empty forwarding blocks.
    static void eliminateUselessBranchesAndBlocks(Function &F) {
        // Early-increment iteration allows basic blocks to be simplified or removed.
        for (BasicBlock &BB : make_early_inc_range(F)) {
            // Replace a conditional branch with identical successors
            // by an unconditional branch.
            if (auto *BI = dyn_cast<BranchInst>(BB.getTerminator())) {
                if (BI->isConditional() && BI->getSuccessor(0) == BI->getSuccessor(1)) {
                    BranchInst::Create(BI->getSuccessor(0), BI);
                    BI->eraseFromParent();
                    Changed = true;
                }
            }

            // Remove an otherwise empty block that only forwards control
            // through an unconditional branch.
            if (auto *BI = dyn_cast<BranchInst>(BB.getTerminator())) {
                if (BI->isUnconditional() && BB.size() == 1) {
                    if (TryToSimplifyUncondBranchFromEmptyBlock(&BB)) {
                        Changed = true;
                    }
                }
            }
        }
    }

    // Walk accesses to one stack slot backwards and remove stores whose
    // values are overwritten or never read.
    static void eliminateDeadStoresForSlot(SmallVectorImpl<Instruction *> &Insts) {
        bool ValueNeeded = false;
        for (Instruction *I : reverse(Insts)) {
            if (isa<LoadInst>(I)) {
                ValueNeeded = true;
            } else if (auto *SI = dyn_cast<StoreInst>(I)) {
                if (!ValueNeeded) {
                    SI->eraseFromParent();
                    Changed = true;
                }
                ValueNeeded = false;
            }
        }
    }

    // Collect direct loads and stores to local stack slots within each
    // basic block and eliminate unnecessary stores.
    static void eliminateDeadStores(Function &F) {
        for (BasicBlock &BB : F) {
            DenseMap<AllocaInst *, SmallVector<Instruction *, 8>> SlotInsts;

            // Group direct load/store instructions by their alloca.
            for (Instruction &I : BB) {
                if (auto *LI = dyn_cast<LoadInst>(&I)) {
                    if (auto *Slot = dyn_cast<AllocaInst>(LI->getPointerOperand())) {
                        SlotInsts[Slot].push_back(&I);
                    }
                } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
                    if (auto *Slot = dyn_cast<AllocaInst>(SI->getPointerOperand())) {
                        SlotInsts[Slot].push_back(&I);
                    }
                }
            }

            // Only analyze slots whose uses are all direct accesses collected above.
            for (auto &[Slot, Insts] : SlotInsts) {
                if (Slot->getNumUses() == Insts.size()) {
                    eliminateDeadStoresForSlot(Insts);
                }
            }
        }
    }

    struct DeadCodeEliminationPass : PassInfoMixin<DeadCodeEliminationPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            // Repeat until no optimization changes the function, since one
            // transformation may expose additional dead instructions.
            do {
                Changed = false;
                eliminateDeadInstructions(F);
                eliminateUselessBranchesAndBlocks(F);
                eliminateDeadStores(F);
            } while (Changed);

            return PreservedAnalyses::none();
        }
    };
} // namespace

/// Registration
PassPluginLibraryInfo getPassPluginInfo() {
    const auto callback = [](PassBuilder &PB) {
        // Make the pass available through:
        // opt -passes=dead-code-elimination
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM, auto) {
                if (Name == "dead-code-elimination") {
                    FPM.addPass(DeadCodeEliminationPass());
                    return true;
                }
                return false;
            });
    };
    return {LLVM_PLUGIN_API_VERSION, "DeadCodeEliminationPass", LLVM_VERSION_STRING, callback};
};

// Entry point used by LLVM to load the pass plugin dynamically.
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getPassPluginInfo();
}

#undef DEBUG_TYPE
