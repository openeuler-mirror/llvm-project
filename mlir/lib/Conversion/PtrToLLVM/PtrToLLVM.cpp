//===- PtrToLLVM.cpp - Ptr to LLVM dialect conversion ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/PtrToLLVM/PtrToLLVM.h"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Dialect/Ptr/IR/PtrOps.h"
#include "mlir/IR/TypeUtilities.h"
#include <type_traits>

using namespace mlir;

//===----------------------------------------------------------------------===//
// ConvertToLLVMPatternInterface implementation
//===----------------------------------------------------------------------===//

namespace {
/// Implement the interface to convert Ptr to LLVM.
struct PtrToLLVMDialectInterface : public ConvertToLLVMPatternInterface {
  using ConvertToLLVMPatternInterface::ConvertToLLVMPatternInterface;
  void loadDependentDialects(MLIRContext *context) const final {
    context->loadDialect<LLVM::LLVMDialect>();
  }

  /// Hook for derived dialect interface to provide conversion patterns
  /// and mark dialect legal for the conversion target.
  void populateConvertToLLVMConversionPatterns(
      ConversionTarget &target, LLVMTypeConverter &converter,
      RewritePatternSet &patterns) const final {
    ptr::populatePtrToLLVMConversionPatterns(converter, patterns);
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// API
//===----------------------------------------------------------------------===//

void mlir::ptr::populatePtrToLLVMConversionPatterns(
    LLVMTypeConverter &converter, RewritePatternSet &patterns) {
  // Add address space conversions.
  converter.addTypeAttributeConversion(
      [&](PtrLikeTypeInterface type, ptr::GenericSpaceAttr memorySpace)
          -> TypeConverter::AttributeConversionResult {
        if (type.getMemorySpace() != memorySpace)
          return TypeConverter::AttributeConversionResult::na();
        return IntegerAttr::get(IntegerType::get(type.getContext(), 32), 0);
      });

  // Add type conversions.
  converter.addConversion([&](ptr::PtrType type) -> Type {
    std::optional<Attribute> maybeAttr =
        converter.convertTypeAttribute(type, type.getMemorySpace());
    auto memSpace =
        maybeAttr ? dyn_cast_or_null<IntegerAttr>(*maybeAttr) : IntegerAttr();
    if (!memSpace)
      return {};
    return LLVM::LLVMPointerType::get(type.getContext(),
                                      memSpace.getValue().getSExtValue());
  });
}

void mlir::ptr::registerConvertPtrToLLVMInterface(DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, ptr::PtrDialect *dialect) {
    dialect->addInterfaces<PtrToLLVMDialectInterface>();
  });
}
