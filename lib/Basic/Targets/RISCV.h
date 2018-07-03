//===--- RISCV.h - Declare RISCV target feature support ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares RISCV TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_RISCV_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_RISCV_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

// RISC-V Target
class RISCVTargetInfo : public TargetInfo {
  void setDataLayout() {
    StringRef Layout;

    if (ABI == "ilp32") {
      if (IsCHERI)
        Layout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128";
      else
        Layout = "e-m:e-p:32:32-i64:64-n32-S128";
    } else if (ABI == "lp64") {
      if (IsCHERI)
        Layout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128";
      else
        Layout = "e-m:e-p:64:64-i64:64-i128:128-n64-S128";
    } else
      llvm_unreachable("Invalid ABI");

    StringRef PurecapOptions = "";
    // Only set globals address space to 200 for cap-table mode
    if (CapabilityABI)
      PurecapOptions = llvm::MCTargetOptions::cheriUsesCapabilityTable()
                           ? "-A200-P200-G200"
                           : "-A200-P200";

    resetDataLayout((Layout + PurecapOptions).str());
  }

protected:
  std::string ABI;
  bool HasM;
  bool HasA;
  bool HasF;
  bool HasD;
  bool HasC;
  bool IsCHERI = false;
  int CapSize = -1;

  void setCapabilityABITypes() {
    IntPtrType = TargetInfo::SignedIntCap;
  }

public:
  RISCVTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple), HasM(false), HasA(false), HasF(false),
        HasD(false), HasC(false) {
    TLSSupported = false;
    LongDoubleWidth = 128;
    LongDoubleAlign = 128;
    LongDoubleFormat = &llvm::APFloat::IEEEquad();
    SuitableAlign = 128;
    WCharType = SignedInt;
    WIntType = UnsignedInt;
  }

  StringRef getABI() const override { return ABI; }
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override { return None; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  bool hasFeature(StringRef Feature) const override;

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  unsigned getIntCapWidth() const override { return CapSize; }
  unsigned getIntCapAlign() const override { return CapSize; }

  uint64_t getCHERICapabilityWidth() const override { return CapSize; }

  uint64_t getCHERICapabilityAlign() const override { return CapSize; }

  uint64_t getPointerWidthV(unsigned AddrSpace) const override {
    return (AddrSpace == 200) ? CapSize : PointerWidth;
  }

  uint64_t getPointerRangeV(unsigned AddrSpace) const override {
    return (AddrSpace == 200) ? getPointerRangeForCHERICapability() : PointerWidth;
  }

  uint64_t getPointerAlignV(unsigned AddrSpace) const override {
    return (AddrSpace == 200) ? CapSize : PointerAlign;
  }

  bool SupportsCapabilities() const override { return IsCHERI; }
};
class LLVM_LIBRARY_VISIBILITY RISCV32TargetInfo : public RISCVTargetInfo {
public:
  RISCV32TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : RISCVTargetInfo(Triple, Opts) {
    IntPtrType = SignedInt;
    PtrDiffType = SignedInt;
    SizeType = UnsignedInt;
    if (Triple.getArch() == llvm::Triple::riscv32_cheri) {
      IsCHERI = true;
      CapSize = 64;
    }
  }

  bool setABI(const std::string &Name) override {
    // TODO: support ilp32f and ilp32d ABIs.
    if (Name == "ilp32") {
      ABI = Name;
      return true;
    }
    if (IsCHERI && Name == "purecap") {
      setCapabilityABITypes();
      CapabilityABI = true;
      ABI = "ilp32";
      return true;
    }
    return false;
  }

  uint64_t getPointerRangeForCHERICapability() const override { return 32; }
};
class LLVM_LIBRARY_VISIBILITY RISCV64TargetInfo : public RISCVTargetInfo {
public:
  RISCV64TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : RISCVTargetInfo(Triple, Opts) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    IntMaxType = Int64Type = SignedLong;
    if (Triple.getArch() == llvm::Triple::riscv64_cheri) {
      IsCHERI = true;
      CapSize = 128;
    }
  }

  bool setABI(const std::string &Name) override {
    // TODO: support lp64f and lp64d ABIs.
    if (Name == "lp64") {
      ABI = Name;
      return true;
    }
    if (IsCHERI && Name == "purecap") {
      setCapabilityABITypes();
      CapabilityABI = true;
      ABI = "lp64";
      return true;
    }
    return false;
  }

  uint64_t getPointerRangeForCHERICapability() const override { return 64; }
};
} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_RISCV_H
