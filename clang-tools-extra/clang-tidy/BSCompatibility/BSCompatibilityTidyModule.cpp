//===--- BSCompatibilityTidyModule.cpp - clang-tidy --------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "../cppcoreguidelines/NarrowingConversionsCheck.h"
#include "NonVoidFunctionReturnVoidCheck.h"

namespace clang::tidy {
namespace BSCompatibility {

class BSCompatibilityModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<NonVoidFunctionReturnVoidCheck>(
        "BSCompatibility-non-void-function-return-void");
  }
};

} // namespace BSCompatibility

// Register the BSCompatibilityModuleRegistry using this statically initsialized variable.
static ClangTidyModuleRegistry::Add<BSCompatibility::BSCompatibilityModule>
    X("BSCompatibility-module", "Adds checks for BiSheng compatibility code constructs.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the BSCompatibilityModule.
volatile int BSCompatibilityModuleAnchorSource = 0;

} // namespace clang::tidy
