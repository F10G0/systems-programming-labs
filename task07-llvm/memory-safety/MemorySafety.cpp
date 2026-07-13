#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#define DEBUG_TYPE "memory-safety"

using namespace llvm;

namespace {
    // Context of the module currently being instrumented.
    // The data layout is needed to compute target-dependent object sizes.
    Module *M;
    const DataLayout *DL;

    // LLVM types used to declare the runtime interfaces.
    // SizeTy corresponds to the target's pointer-sized integer type.
    struct RuntimeTypes {
        Type *VoidTy;
        Type *PtrTy;
        Type *SizeTy;
    };

    // Instructions relevant to memory instrumentation.
    // They are collected in one traversal to avoid scanning the function
    // separately for each instrumentation stage.
    struct CollectedInstructions {
        SmallVector<CallInst *, 16> Calls;
        SmallVector<Instruction *, 32> Accesses;
        SmallVector<AllocaInst *, 16> Allocations;
    };

    // Insert a runtime function declaration into the current module when
    // no compatible declaration exists yet.
    FunctionCallee declareFunc(StringRef Name, Type *RetTy, ArrayRef<Type *> ArgTys) {
        FunctionType *FnTy = FunctionType::get(RetTy, ArgTys, false);
        return M->getOrInsertFunction(Name, FnTy);
    }

    // Initialize the target-dependent information required by the pass.
    RuntimeTypes initContext(Function &F) {
        M = F.getParent();
        DL = &M->getDataLayout();

        LLVMContext &Ctx = F.getContext();
        Type *VoidTy = Type::getVoidTy(Ctx);
        Type *AddrTy = PointerType::getUnqual(Ctx);
        Type *IntPtrTy = DL->getIntPtrType(Ctx);
        return {VoidTy, AddrTy, IntPtrTy};
    }

    // Collect all instructions that will later be transformed.
    // Calls are used for malloc/free replacement, allocations for stack
    // registration, and accesses for load/store bounds checks.
    CollectedInstructions collectInstructions(Function &F) {
        CollectedInstructions Insts;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    Insts.Calls.push_back(CI);
                } else if (auto *AI = dyn_cast<AllocaInst>(&I)) {
                    Insts.Allocations.push_back(AI);
                } else if (isa<LoadInst>(&I) || isa<StoreInst>(&I)) {
                    Insts.Accesses.push_back(&I);
                }
            }
        }
        return Insts;
    }

    // Redirect heap allocation and deallocation through the runtime so that
    // allocated ranges and freed blocks can be tracked.
    void replaceMemoryCalls(ArrayRef<CallInst *> Calls, const RuntimeTypes &Types) {
        FunctionCallee MallocFn = declareFunc("__runtime_malloc", Types.PtrTy, {Types.SizeTy});
        FunctionCallee FreeFn = declareFunc("__runtime_free", Types.VoidTy, {Types.PtrTy});

        for (CallInst *CI : Calls) {
            // Indirect calls have no directly available Function object and
            // are therefore left unchanged.
            if (Function *Fn = CI->getCalledFunction()) {
                if (Fn->getName() == "malloc") {
                    Value *Size = CI->getArgOperand(0);

                    // Insert the wrapper before the original call and redirect
                    // all users of the original return value to the new call.
                    CallInst *NewCI = CallInst::Create(MallocFn, {Size}, "", CI);
                    CI->replaceAllUsesWith(NewCI);
                    CI->eraseFromParent();
                } else if (Fn->getName() == "free") {
                    Value *Ptr = CI->getArgOperand(0);

                    // free returns void, so the original call has no uses that
                    // need to be redirected.
                    CallInst::Create(FreeFn, {Ptr}, "", CI);
                    CI->eraseFromParent();
                }
            }
        }
    }

    // Register every stack object with the runtime. The registration call is
    // inserted after the alloca because it needs the pointer produced by it.
    void registerStackAllocations(ArrayRef<AllocaInst *> Allocations, const RuntimeTypes &Types) {
        FunctionCallee RegisterFn = declareFunc("__runtime_alloc", Types.VoidTy, {Types.PtrTy, Types.SizeTy});

        for (AllocaInst *AI : Allocations) {
            Type *ObjTy = AI->getAllocatedType();

            // Allocation size accounts for the target's layout and alignment
            // requirements for the stack object.
            uint64_t Bytes = DL->getTypeAllocSize(ObjTy).getFixedValue();
            Value *Size = ConstantInt::get(Types.SizeTy, Bytes);

            CallInst::Create(RegisterFn, {AI, Size}, "", AI->getNextNode());
        }
    }

    // Insert a runtime validation call immediately before every load and
    // store so that an invalid address is rejected before it is dereferenced.
    void checkMemoryAccesses(ArrayRef<Instruction *> Accesses, const RuntimeTypes &Types) {
        FunctionCallee CheckFn = declareFunc("__runtime_check_addr", Types.VoidTy, {Types.PtrTy, Types.SizeTy});

        for (Instruction *I : Accesses) {
            Value *Ptr = nullptr;
            Type *ValTy = nullptr;

            // A load obtains its accessed type from the loaded result, while
            // a store obtains it from the value being written.
            if (auto *LI = dyn_cast<LoadInst>(I)) {
                Ptr = LI->getPointerOperand();
                ValTy = LI->getType();
            } else if (auto *SI = dyn_cast<StoreInst>(I)) {
                Ptr = SI->getPointerOperand();
                ValTy = SI->getValueOperand()->getType();
            }

            // Store size represents the number of bytes actually accessed by
            // the load or store instruction.
            uint64_t Bytes = DL->getTypeStoreSize(ValTy).getFixedValue();
            Value *Size = ConstantInt::get(Types.SizeTy, Bytes);

            CallInst::Create(CheckFn, {Ptr, Size}, "", I);
        }
    }

    struct MemorySafetyPass : PassInfoMixin<MemorySafetyPass> {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            // Instrumenting the runtime itself would recursively insert checks
            // into the functions responsible for performing those checks.
            if (F.getName().startswith("__runtime_")) {
                return PreservedAnalyses::all();
            }

            RuntimeTypes Types = initContext(F);
            CollectedInstructions Insts = collectInstructions(F);

            // Apply the three independent instrumentation stages using the
            // instruction lists collected before any IR modification.
            replaceMemoryCalls(Insts.Calls, Types);
            registerStackAllocations(Insts.Allocations, Types);
            checkMemoryAccesses(Insts.Accesses, Types);

            return PreservedAnalyses::none();
        }

        // Run even on functions marked with optnone.
        static bool isRequired() { return true; }
    };
} // namespace

/// Registration
PassPluginLibraryInfo getPassPluginInfo() {
    const auto callback = [](PassBuilder &PB) {
        // Register the textual pipeline name used by:
        // opt -passes=memory-safety
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM, auto) {
                if (Name == "memory-safety") {
                    FPM.addPass(MemorySafetyPass());
                    return true;
                }
                return false;
            });
    };

    return {LLVM_PLUGIN_API_VERSION, "MemorySafetyPass", LLVM_VERSION_STRING, callback};
};

// Dynamic entry point queried by LLVM when loading the pass plugin.
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getPassPluginInfo();
}

#undef DEBUG_TYPE
