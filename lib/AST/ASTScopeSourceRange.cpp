//===--- ASTScopeSourceRange.cpp - Swift Object-Oriented AST Scope --------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
///
/// This file implements the source range queries for the ASTScopeImpl ontology.
///
//===----------------------------------------------------------------------===//
#include "swift/AST/ASTScope.h"

#include "swift/AST/ASTContext.h"
#include "swift/AST/ASTWalker.h"
#include "swift/AST/Decl.h"
#include "swift/AST/Expr.h"
#include "swift/AST/Initializer.h"
#include "swift/AST/LazyResolver.h"
#include "swift/AST/Module.h"
#include "swift/AST/ParameterList.h"
#include "swift/AST/Pattern.h"
#include "swift/AST/Stmt.h"
#include "swift/AST/TypeRepr.h"
#include "swift/Basic/STLExtras.h"
#include "swift/Parse/Lexer.h"
#include "llvm/Support/Compiler.h"
#include <algorithm>

using namespace swift;
using namespace ast_scope;

static SourceLoc getStartOfFirstParam(ClosureExpr *closure);
static SourceLoc getEndLocEvenWhenRBraceIsMissing(const SourceManager &,
                                                  SourceLoc endLoc);

SourceRange ASTScopeImpl::widenSourceRangeForIgnoredASTNodes(
    const SourceRange range) const {
  if (range.isInvalid())
    return sourceRangeOfIgnoredASTNodes;
  auto r = range;
  if (sourceRangeOfIgnoredASTNodes.isValid())
    r.widen(sourceRangeOfIgnoredASTNodes);
  return r;
}

SourceRange
ASTScopeImpl::widenSourceRangeForChildren(const SourceRange range,
                                          const bool omitAssertions) const {
  if (getChildren().empty()) {
    assert(omitAssertions || range.Start.isValid());
    return range;
  }
  const auto childStart =
      getChildren().front()->getSourceRangeOfScope(omitAssertions).Start;
  const auto childEnd =
      getChildren().back()->getSourceRangeOfScope(omitAssertions).End;
  auto childRange = SourceRange(childStart, childEnd);
  assert(omitAssertions || childRange.isValid());

  if (range.isInvalid())
    return childRange;
  auto r = range;
  r.widen(childRange);
  return r;
}

bool ASTScopeImpl::checkSourceRangeAfterExpansion() const {
  assert((getSourceRangeOfThisASTNode().isValid() || !getChildren().empty()) &&
         "need to be able to find source range");
  assert(verifyThatChildrenAreContainedWithin(getSourceRangeOfScope()) &&
         "Search will fail");
  return true;
}

#pragma mark validation

bool ASTScopeImpl::hasValidSourceRange() const {
  const auto sourceRange = getSourceRangeOfScope();
  return sourceRange.Start.isValid() && sourceRange.End.isValid() &&
         !getSourceManager().isBeforeInBuffer(sourceRange.End,
                                              sourceRange.Start);
}

bool ASTScopeImpl::hasValidSourceRangeOfIgnoredASTNodes() const {
  return sourceRangeOfIgnoredASTNodes.isValid();
}

bool ASTScopeImpl::precedesInSource(const ASTScopeImpl *next) const {
  if (!hasValidSourceRange() || !next->hasValidSourceRange())
    return false;
  return !getSourceManager().isBeforeInBuffer(
      next->getSourceRangeOfScope().Start, getSourceRangeOfScope().End);
}

bool ASTScopeImpl::verifyThatChildrenAreContainedWithin(
    const SourceRange range) const {
  // assumes children are already in order
  if (getChildren().empty())
    return true;
  const SourceRange rangeOfChildren =
      SourceRange(getChildren().front()->getSourceRangeOfScope().Start,
                  getChildren().back()->getSourceRangeOfScope().End);
  if (getSourceManager().rangeContains(range, rangeOfChildren))
    return true;
  auto &out = verificationError() << "children not contained in its parent\n";
  if (getChildren().size() == 1) {
    out << "\n***Only Child node***\n";
    getChildren().front()->print(out);
  } else {
    out << "\n***First Child node***\n";
    getChildren().front()->print(out);
    out << "\n***Last Child node***\n";
    getChildren().back()->print(out);
  }
  out << "\n***Parent node***\n";
  this->print(out);
  abort();
}

bool ASTScopeImpl::verifyThatThisNodeComeAfterItsPriorSibling() const {
  auto priorSibling = getPriorSibling();
  if (!priorSibling)
    return true;
  if (priorSibling.get()->precedesInSource(this))
    return true;
  auto &out = verificationError() << "unexpected out-of-order nodes\n";
  out << "\n***Penultimate child node***\n";
  priorSibling.get()->print(out);
  out << "\n***Last Child node***\n";
  print(out);
  out << "\n***Parent node***\n";
  getParent().get()->print(out);
  //  llvm::errs() << "\n\nsource:\n"
  //               << getSourceManager()
  //                      .getRangeForBuffer(
  //                          getSourceFile()->getBufferID().getValue())
  //                      .str();
  llvm_unreachable("unexpected out-of-order nodes");
  return false;
}

NullablePtr<ASTScopeImpl> ASTScopeImpl::getPriorSibling() const {
  auto parent = getParent();
  if (!parent)
    return nullptr;
  auto const &siblingsAndMe = parent.get()->getChildren();
  // find myIndex, which is probably the last one
  int myIndex = -1;
  for (int i = siblingsAndMe.size() - 1; i >= 0; --i) {
    if (siblingsAndMe[i] == this) {
      myIndex = i;
      break;
    }
  }
  assert(myIndex != -1 && "I have been disowned!");
  if (myIndex == 0)
    return nullptr;
  return siblingsAndMe[myIndex - 1];
}

#pragma mark getSourceRangeOfThisASTNode

SourceRange SpecializeAttributeScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return specializeAttr->getRange();
}

SourceRange AbstractFunctionBodyScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return decl->getBodySourceRange();
}

SourceRange TopLevelCodeScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return decl->getSourceRange();
}

SourceRange SubscriptDeclScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return decl->getSourceRange();
}

SourceRange
EnumElementScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  return decl->getSourceRange();
}

SourceRange WholeClosureScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return closureExpr->getSourceRange();
}

SourceRange AbstractStmtScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return getStmt()->getSourceRange();
}

SourceRange DefaultArgumentInitializerScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  if (auto *dv = decl->getDefaultValue())
    return dv->getSourceRange();
  return SourceRange();
}

SourceRange PatternEntryDeclScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  // TODO: Once rdar://53627317 is accomplished, the following may be able to be
  // simplified.
  if (!getChildren().empty()) { // why needed???
    bool hasOne = false;
    getPattern()->forEachVariable([&](VarDecl *) { hasOne = true; });
    if (!hasOne)
      return SourceRange(); // just the init
    if (!getPatternEntry().getInit())
      return SourceRange(); // just the var decls
  }
  return getPatternEntry().getSourceRange();
}

SourceRange PatternEntryInitializerScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  // See rdar://53921703
  // Note: grep for "When the initializer is removed we don't actually clear the
  // pointer" because we do!
  return initAsWrittenWhenCreated->getSourceRange();
}

SourceRange
VarDeclScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  const auto br = decl->getBracesRange();
  return br.isValid() ? br : decl->getSourceRange();
}

SourceRange GenericParamScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  auto nOrE = holder;
  // A protocol's generic parameter list is not written in source, and
  // is visible from the start of the body.
  if (auto *protoDecl = dyn_cast<ProtocolDecl>(nOrE))
    return SourceRange(protoDecl->getBraces().Start, protoDecl->getEndLoc());
  auto startLoc = paramList->getSourceRange().Start;
  if (startLoc.isInvalid())
    startLoc = holder->getStartLoc();
  return SourceRange(startLoc, holder->getEndLoc());
}

SourceRange ASTSourceFileScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  if (auto bufferID = SF->getBufferID()) {
    auto charRange = getSourceManager().getRangeForBuffer(*bufferID);
    return SourceRange(charRange.getStart(), charRange.getEnd());
  }

  if (SF->Decls.empty())
    return SourceRange();

  // Use the source ranges of the declarations in the file.
  return SourceRange(SF->Decls.front()->getStartLoc(),
                     SF->Decls.back()->getEndLoc());
}

SourceRange GenericTypeOrExtensionScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return portion->getChildlessSourceRangeOf(this, omitAssertions);
}

SourceRange GenericTypeOrExtensionWholePortion::getChildlessSourceRangeOf(
    const GenericTypeOrExtensionScope *scope, const bool omitAssertions) const {
  auto *d = scope->getDecl();
  auto r = d->getSourceRangeIncludingAttrs();
  if (r.Start.isValid()) {
    assert(r.End.isValid());
    return r;
  }
  return d->getSourceRange();
}

SourceRange GenericTypeOrExtensionWherePortion::getChildlessSourceRangeOf(
    const GenericTypeOrExtensionScope *scope, const bool omitAssertions) const {
  return scope->getGenericContext()->getTrailingWhereClause()->getSourceRange();
}

SourceRange IterableTypeBodyPortion::getChildlessSourceRangeOf(
    const GenericTypeOrExtensionScope *scope, const bool omitAssertions) const {
  auto *d = scope->getDecl();
  if (auto *nt = dyn_cast<NominalTypeDecl>(d))
    return nt->getBraces();
  if (auto *e = dyn_cast<ExtensionDecl>(d))
    return e->getBraces();
  if (omitAssertions)
    return SourceRange();
  llvm_unreachable("No body!");
}

SourceRange AbstractFunctionDeclScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  // For a get/put accessor	 all of the parameters are implicit, so start
  // them at the start location of the accessor.
  auto r = decl->getSourceRangeIncludingAttrs();
  if (r.Start.isValid()) {
    assert(r.End.isValid());
    return r;
  }
  return decl->getBodySourceRange();
}

SourceRange ParameterListScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  const auto rangeForGoodInput = getSourceRangeOfEnclosedParams(omitAssertions);
  auto r = SourceRange(rangeForGoodInput.Start,
                       fixupEndForBadInput(rangeForGoodInput));
  assert(getSourceManager().rangeContains(
             getParent().get()->getSourceRangeOfThisASTNode(true), r) &&
         "Parameters not within function?!");
  return r;
}

SourceLoc ParameterListScope::fixupEndForBadInput(
    const SourceRange rangeForGoodInput) const {
  const auto s = rangeForGoodInput.Start;
  const auto e = rangeForGoodInput.End;
  return getSourceManager().isBeforeInBuffer(s, e) ? e : s;
}

SourceRange ForEachPatternScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  // The scope of the pattern extends from the 'where' expression (if present)
  // until the end of the body.
  if (stmt->getWhere())
    return SourceRange(stmt->getWhere()->getStartLoc(),
                       stmt->getBody()->getEndLoc());

  // Otherwise, scope of the pattern covers the body.
  return stmt->getBody()->getSourceRange();
}

SourceRange
CatchStmtScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  // The scope of the pattern extends from the 'where' (if present)
  // to the end of the body.
  if (stmt->getGuardExpr())
    return SourceRange(stmt->getWhereLoc(), stmt->getBody()->getEndLoc());

  // Otherwise, the scope of the pattern encompasses the body.
  return stmt->getBody()->getSourceRange();
}
SourceRange
CaseStmtScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  // The scope of the case statement begins at the first guard expression,
  // if there is one, and extends to the end of the body.
  // FIXME: Figure out what to do about multiple pattern bindings. We might
  // want a more restrictive rule in those cases.
  for (const auto &caseItem : stmt->getCaseLabelItems()) {
    if (auto guardExpr = caseItem.getGuardExpr())
      return SourceRange(guardExpr->getStartLoc(),
                         stmt->getBody()->getEndLoc());
  }

  // Otherwise, it covers the body.
  return stmt->getBody()
      ->getSourceRange(); // The scope of the case statement begins
}

SourceRange
BraceStmtScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  // The brace statements that represent closures start their scope at the
  // 'in' keyword, when present.
  if (auto closure = parentClosureIfAny()) {
    if (closure.get()->getInLoc().isValid())
      return SourceRange(closure.get()->getInLoc(), stmt->getEndLoc());
  }
  return stmt->getSourceRange();
}

SourceRange ConditionalClauseScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  // From the start of this particular condition to the start of the
  // then/body part.
  const auto startLoc = getStmtConditionElement().getStartLoc();
  return startLoc.isValid()
         ? SourceRange(startLoc, endLoc)
         : SourceRange(endLoc);
}

SourceRange ConditionalClausePatternUseScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  // For a guard continuation, the scope extends from the end of the 'else'
  // to the end of the continuation.
  return SourceRange(startLoc);
}

SourceRange
CaptureListScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  auto *const closure = expr->getClosureBody();
  return SourceRange(expr->getStartLoc(), getStartOfFirstParam(closure));
}

SourceRange ClosureParametersScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  if (!omitAssertions)
    assert(closureExpr->getInLoc().isValid() &&
           "We don't create these if no in loc");
  return SourceRange(getStartOfFirstParam(closureExpr),
                     closureExpr->getInLoc());
}

SourceRange
ClosureBodyScope::getSourceRangeOfThisASTNode(const bool omitAssertions) const {
  if (closureExpr->getInLoc().isValid())
    return SourceRange(closureExpr->getInLoc(), closureExpr->getEndLoc());

  return closureExpr->getSourceRange();
}

SourceRange AttachedPropertyWrapperScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return sourceRangeWhenCreated;
}

SourceRange LookupParentDiversionScope::getSourceRangeOfThisASTNode(
    const bool omitAssertions) const {
  return SourceRange(startLoc);
}

#pragma mark source range caching

SourceRange
ASTScopeImpl::getSourceRangeOfScope(const bool omitAssertions) const {
  if (!isSourceRangeCached(omitAssertions))
    cacheSourceRangeOfMeAndDescendants(omitAssertions);
  return *cachedSourceRange;
}

bool ASTScopeImpl::isSourceRangeCached(const bool omitAssertions) const {
  const bool isCached = cachedSourceRange.hasValue();
  assert(omitAssertions || isCached || ensureNoAncestorsSourceRangeIsCached());
  return isCached;
}

bool ASTScopeImpl::ensureNoAncestorsSourceRangeIsCached() const {
  if (const auto *const p = getParent().getPtrOrNull()) {
    auto r = !p->isSourceRangeCached(true) &&
             p->ensureNoAncestorsSourceRangeIsCached();
    if (!r)
      llvm_unreachable("found a violation");
    return true;
  }
  return true;
}

void ASTScopeImpl::cacheSourceRangeOfMeAndDescendants(
    const bool omitAssertions) const {
  // In order to satisfy the invariant that, if my range is uncached,
  // my parent's range is uncached, (which is needed to optimize invalidation
  // by obviating the need to uncache all the way to the root every time),
  // when caching a range, must ensure all children's ranges are cached.
  for (auto *c : getChildren())
    c->cacheSourceRangeOfMeAndDescendants(omitAssertions);

  cachedSourceRange = getUncachedSourceRange(omitAssertions);
}

bool ASTScopeImpl::checkLazySourceRange(const SourceRange expandedRange) const {
  if (!getASTContext().LangOpts.LazyASTScopes)
    return true;
  const auto unexpandedRange = sourceRangeForDeferredExpansion();
  if (unexpandedRange.isInvalid() || expandedRange.isInvalid())
    return true;
  if (unexpandedRange == expandedRange)
    return true;

  auto b = getChildren().back()->getUncachedSourceRange();
  llvm::errs() << "*** Lazy range problem. Parent: ***\n";
  unexpandedRange.print(llvm::errs(), getSourceManager(), false);
  llvm::errs() << "\n*** vs last child: ***\n";
  b.print(llvm::errs(), getSourceManager(), false);
  llvm::errs() << "\n";
  print(llvm::errs(), 0, false);
  llvm::errs() << "\n";

  return false;
}

SourceRange
ASTScopeImpl::getUncachedSourceRange(const bool omitAssertions) const {
  const auto childlessRange = getSourceRangeOfThisASTNode(omitAssertions);
  const auto rangeIncludingIgnoredNodes =
      widenSourceRangeForIgnoredASTNodes(childlessRange);
  const auto uncachedSourceRange =
      widenSourceRangeForChildren(rangeIncludingIgnoredNodes, omitAssertions);
  assert(
      omitAssertions ||
      checkLazySourceRange(uncachedSourceRange) &&
          "Lazy scopes must have compatible ranges before and after expansion");
  return uncachedSourceRange;
}

void ASTScopeImpl::clearCachedSourceRangesOfMeAndAncestors() {
  // An optimization: if my range isn't cached, my ancestors must not be
  if (!isSourceRangeCached())
    return;
  cachedSourceRange = None;
  if (auto p = getParent())
    p.get()->clearCachedSourceRangesOfMeAndAncestors();
}

#pragma mark ignored nodes and compensating for InterpolatedStringLiteralExprs and EditorPlaceHolders

namespace {
// TODO: try testing the last token to see if it is an
// InterpolatedStringLiteralExpr or an EditorPlaceHolder.
// Then relex to find the end.
// (Should use CharSourceRanges but they would perform an extra addition.)
class EffectiveEndFinder : public ASTWalker {
  SourceLoc end;
  const SourceManager &SM;

public:
  EffectiveEndFinder(const SourceManager &SM) : SM(SM) {}

  std::pair<bool, Expr *> walkToExprPre(Expr *E) {
    if (!E)
      return {true, E};
    if (auto *isl = dyn_cast<InterpolatedStringLiteralExpr>(E)) {
      const auto e = isl->getTrailingQuoteLoc();
      if (e.isValid() && (end.isInvalid() || SM.isBeforeInBuffer(end, e)))
        end = e;
    } else if (auto *epl = dyn_cast<EditorPlaceholderExpr>(E)) {
      if (end.isInvalid() ||
          SM.isBeforeInBuffer(end, epl->getTrailingAngleBracketLoc()))
        end = epl->getTrailingAngleBracketLoc();
    }
    return ASTWalker::walkToExprPre(E);
  }
  SourceLoc getEffectiveEndLoc() const { return end; }
};
} // namespace

SourceRange ASTScopeImpl::getEffectiveSourceRange(const ASTNode n) const {
  if (const auto *d = n.dyn_cast<Decl *>())
    return d->getSourceRange();
  if (const auto *s = n.dyn_cast<Stmt *>())
    return s->getSourceRange();
  auto *e = n.dyn_cast<Expr *>();
  assert(e);
  EffectiveEndFinder finder(getSourceManager());
  e->walk(finder);
  return SourceRange(e->getLoc(), finder.getEffectiveEndLoc().isValid()
                                      ? finder.getEffectiveEndLoc()
                                      : e->getEndLoc());
}

/// Some nodes (e.g. the error expression) cannot possibly contain anything to
/// be looked up and if included in a parent scope's source range would expand
/// it beyond an ancestor's source range. But if the ancestor is expanded
/// lazily, we check that its source range does not change when expanding it,
/// and this check would fail.
static bool sourceRangeWouldInterfereWithLaziness(const ASTNode n) {
  return n.isExpr(ExprKind::Error);
}

static bool
shouldIgnoredASTNodeSourceRangeWidenEnclosingScope(const ASTNode n) {
  if (n.isDecl(DeclKind::Var)) {
    // The pattern scopes will include the source ranges for VarDecls.
    // Using its range here would cause a pattern initializer scope's range
    // to overlap the pattern use scope's range.
    return false;
  }
  if (sourceRangeWouldInterfereWithLaziness(n))
    return false;
  return true;
}

void ASTScopeImpl::widenSourceRangeForIgnoredASTNode(const ASTNode n) {
  if (!shouldIgnoredASTNodeSourceRangeWidenEnclosingScope(n))
    return;

  SourceRange r = getEffectiveSourceRange(n);
  if (r.isInvalid())
    return;
  if (sourceRangeOfIgnoredASTNodes.isInvalid())
    sourceRangeOfIgnoredASTNodes = r;
  else
    sourceRangeOfIgnoredASTNodes.widen(r);
}

static SourceLoc getStartOfFirstParam(ClosureExpr *closure) {
  if (auto *parms = closure->getParameters()) {
    if (parms->size())
      return parms->get(0)->getStartLoc();
  }
  if (closure->getInLoc().isValid())
    return closure->getInLoc();
  if (closure->getBody())
    return closure->getBody()->getLBraceLoc();
  return closure->getStartLoc();
}

#pragma mark getSourceRangeOfEnclosedParams

SourceRange
ASTScopeImpl::getSourceRangeOfEnclosedParams(const bool omitAssertions) const {
  return getParent().get()->getSourceRangeOfEnclosedParams(omitAssertions);
}

SourceRange
EnumElementScope::getSourceRangeOfEnclosedParams(bool omitAssertions) const {
  auto *pl = decl->getParameterList();
  return pl ? pl->getSourceRange() : SourceRange();
}

SourceRange SubscriptDeclScope::getSourceRangeOfEnclosedParams(
    const bool omitAssertions) const {
  auto r = SourceRange(decl->getIndices()->getLParenLoc(), decl->getEndLoc());
  // Because of "subscript(x: MyStruct#^PARAM_1^#) -> Int { return 0 }"
  // Cannot just use decl->getEndLoc()
  r.widen(decl->getIndices()->getRParenLoc());
  return r;
}

SourceRange AbstractFunctionDeclScope::getSourceRangeOfEnclosedParams(
    const bool omitAssertions) const {
  const auto s = getParamsSourceLoc(decl);
  const auto e = getSourceRangeOfThisASTNode(omitAssertions).End;
  return s.isInvalid() || e.isInvalid() ? SourceRange() : SourceRange(s, e);
}

SourceLoc
AbstractFunctionDeclScope::getParamsSourceLoc(AbstractFunctionDecl *decl) {
  if (auto *c = dyn_cast<ConstructorDecl>(decl))
    return c->getParameters()->getLParenLoc();

  if (auto *dd = dyn_cast<DestructorDecl>(decl))
    return dd->getNameLoc();

  auto *fd = cast<FuncDecl>(decl);
  // clang-format off
  return isa<AccessorDecl>(fd) ? fd->getLoc()
       : fd->isDeferBody()     ? fd->getNameLoc()
       :                         fd->getParameters()->getLParenLoc();
  // clang-format on
}

# pragma mark unexpected source range

// If right brace is missing, the source range of the body will end
// at the last token, which may be a one of the special cases below.
static SourceLoc getEndLocEvenWhenRBraceIsMissing(const SourceManager &SM,
                                                  const SourceLoc endLoc) {
  const auto tok = Lexer::getTokenAtLocation(SM, endLoc);
  switch (tok.getKind()) {
  default:
    return endLoc;
  case tok::string_literal:
    return tok.getRange().getEnd();
  case tok::identifier:
    return Identifier::isEditorPlaceholder(tok.getText())
               ? tok.getRange().getEnd()
               : endLoc;
  }
}

SourceRange ASTScopeImpl::sourceRangeForDeferredExpansion() const {
  return SourceRange();
}
SourceRange IterableTypeScope::sourceRangeForDeferredExpansion() const {
  return portion->sourceRangeForDeferredExpansion(this);
}
SourceRange
Portion::sourceRangeForDeferredExpansion(const IterableTypeScope *) const {
  return SourceRange();
}



SourceRange IterableTypeBodyPortion::sourceRangeForDeferredExpansion(
    const IterableTypeScope *s) const {
  const auto bracesRange = getChildlessSourceRangeOf(s, false);
  const auto &SM = s->getSourceManager();
  return SourceRange(bracesRange.Start,
                     getEndLocEvenWhenRBraceIsMissing(SM, bracesRange.End));
}
