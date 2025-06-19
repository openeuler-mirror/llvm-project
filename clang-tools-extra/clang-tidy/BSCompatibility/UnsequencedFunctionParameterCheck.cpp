//===--- NonVoidFunctionReturnVoidCheck.cpp - clang-tidy ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnsequencedFunctionParameterCheck.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang::tidy::BSCompatibility {

void UnsequencedFunctionParameterCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(callExpr(hasAnyArgument(callExpr())).bind("callee"), this);
}

void UnsequencedFunctionParameterCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Callee = Result.Nodes.getNodeAs<clang::CallExpr>("callee");
  if (!Callee || Callee->getNumArgs() < 2)
    return;

  ASTContext *Context = Result.Context;
  SourceManager &SM = *Result.SourceManager;
  const LangOptions &LangOpts = Context->getLangOpts();

  std::vector<const CallExpr *> CallArgs;
  std::vector<FixItHint> FixIts;

  for (size_t i = 0; i < Callee->getNumArgs(); ++i) {
    if (const auto *CallArg = dyn_cast<CallExpr>(Callee->getArg(i))) {
      CallArgs.push_back(CallArg);
    }
  }

  if (CallArgs.size() < 2)
    return;

  std::string TempDecls = "\n";
  std::vector<std::string> TempVarNames;
  int TempCounter = 0;

  // Get the location for variable insertions.
  SourceLocation InsertLoc = findSafeInsertionPoint(Callee, SM, *Context);

  // Create insertions.
  for (const CallExpr *Call : CallArgs) {
    std::string VarName = (Twine("__temp_") + Twine(GlobalTempCounter) +
                           Twine("_") + Twine(TempCounter++))
                              .str();
    TempVarNames.push_back(VarName);

    QualType ReturnType = Call->getType();
    std::string TypeStr = ReturnType.getAsString();

    SourceRange CallRange = Call->getSourceRange();
    std::string CallText =
        Lexer::getSourceText(CharSourceRange::getTokenRange(CallRange), SM,
                             LangOpts)
            .str();

    TempDecls += "\t" + TypeStr + " " + VarName + " = " + CallText + ";\n";
  }

  if (!TempDecls.empty()) {
    FixIts.push_back(FixItHint::CreateInsertion(InsertLoc, TempDecls));
  }

  // Create replacements.
  TempCounter = 0;
  GlobalTempCounter++;
  for (size_t i = 0; i < Callee->getNumArgs(); ++i) {
    if (isa<CallExpr>(Callee->getArg(i))) {
      const Expr *Arg = Callee->getArg(i);
      FixIts.push_back(FixItHint::CreateReplacement(
          Arg->getSourceRange(), TempVarNames[TempCounter++]));
    }
  }

  // Print diaginfo.
  auto Diag =
      diag(Callee->getBeginLoc(), "Function calls as arguments are unsequenced "
                                  "and may cause dependency issues");

  for (const FixItHint &Fix : FixIts) {
    Diag << Fix;
  }
}

SourceLocation UnsequencedFunctionParameterCheck::findSafeInsertionPoint(
    const CallExpr *Callee, SourceManager &SM, ASTContext &Context) {
  auto Parents = Context.getParents(*Callee);
  if (Parents.empty()) {
    // Insert at line start if there are no parents.
    return findLineStart(Callee->getBeginLoc(), SM);
  }

  const CompoundStmt *CS = nullptr;
  for (const auto &Parent : Parents) {
    if (const auto *CompStmt = Parent.get<CompoundStmt>()) {
      CS = CompStmt;
      break;
    }
  }

  if (!CS) {
    return findLineStart(Callee->getBeginLoc(), SM);
  }

  for (auto I = CS->body_begin(); I != CS->body_end(); ++I) {
    if (*I == Callee) {
      if (I != CS->body_begin()) {
        // insert after ;
        Stmt *Prev = *(I - 1);
        SourceLocation EndLoc = Prev->getEndLoc();
        SourceLocation AfterSemi = Lexer::findLocationAfterToken(
            EndLoc, tok::semi, SM, Context.getLangOpts(), false);
        if (AfterSemi.isValid()) {
          return AfterSemi;
        }
        return EndLoc.getLocWithOffset(1);
      } else {
        // insert after {
        SourceLocation LBrac = CS->getLBracLoc();
        return LBrac.getLocWithOffset(1);
      }
    }
  }
  return CS->getLBracLoc().getLocWithOffset(1);
}

SourceLocation
UnsequencedFunctionParameterCheck::findLineStart(SourceLocation Loc,
                                                 SourceManager &SM) {
  if (Loc.isInvalid())
    return Loc;

  unsigned Line = SM.getSpellingLineNumber(Loc);
  FileID FID = SM.getFileID(Loc);
  SourceLocation LineStart = SM.translateLineCol(FID, Line, 1);

  return LineStart.isValid() ? LineStart : Loc;
}

} // namespace clang::tidy::BSCompatibility
