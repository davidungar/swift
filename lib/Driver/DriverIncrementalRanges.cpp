//===------ DriverIncrementalRanges.cpp ------------------------------------==//
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

#include "swift/Driver/DriverIncrementalRanges.h"

#include "swift/AST/DiagnosticEngine.h"
#include "swift/Basic/Diff.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <algorithm>

using namespace swift;
using namespace incremental_ranges;

//==============================================================================
// MARK: UnparsedRangesForEachPrimary
//==============================================================================


void UnparsedRangesForEachPrimary::addRangesFromPath(
    const StringRef primaryPath, const StringRef unparsedRangesPath,
    DiagnosticEngine &diags) {
  auto errorOrBuffer = llvm::MemoryBuffer::getFile(unparsedRangesPath);
  if (std::error_code ec = errorOrBuffer.getError()) {
    llvm::errs() << "WARNING could not read: " << unparsedRangesPath
                 << " error " << ec.message() << "\n";
    return;
  }
  if (addRangesFromBuffer(primaryPath, *errorOrBuffer->get()))
    llvm::errs() << "WARNING could not read: " << unparsedRangesPath;
}

bool UnparsedRangesForEachPrimary::addRangesFromBuffer(
    const StringRef primaryPath, const llvm::MemoryBuffer &buffer) {
  RangesByFilename rangesByNonprimaryFile;
  if (!buffer.getBuffer().startswith(unparsedRangesFileHeader))
    return true;

  llvm::yaml::Input yamlReader(llvm::MemoryBufferRef(buffer), nullptr);
  yamlReader >> rangesByNonprimaryFile;
  if (yamlReader.error())
    return true;
  addNonprimaryRangesForPrimary(primaryPath, std::move(rangesByNonprimaryFile));
  return false;
}

void UnparsedRangesForEachPrimary::addNonprimaryRangesForPrimary(
    const StringRef primaryPath, RangesByFilename rangesByNonprimaryFile) {
  const auto inserted = rangesByNonprimaryByPrimary.insert(
      {primaryPath.str(), std::move(rangesByNonprimaryFile)});
  assert(inserted.second && "Should not have duplicate primaries.");
}

void UnparsedRangesForEachPrimary::dump() {
  llvm::errs() << "\n*** unparsed ranges: ***\n";
  llvm::yaml::Output dumper(llvm::errs());
  dumper << rangesByNonprimaryByPrimary;
}

//==============================================================================
// MARK: ChangedSourceRangesForEachPrimary
//==============================================================================

void ChangedSourceRangesForEachPrimary::addChanges(StringRef primaryPath, StringRef compiledSourcePath, DiagnosticEngine& diags) {
  auto errorOrCurrentBuffer = llvm::MemoryBuffer::getFile(primaryPath);
  if (std::error_code ec = errorOrCurrentBuffer.getError()) {
    llvm::errs() << "WARNING: could not read: " << primaryPath
    << " error " << ec.message();
    return;
  }
  auto errorOrPreviousBuffer = llvm::MemoryBuffer::getFile(compiledSourcePath);
  if (std::error_code ec = errorOrPreviousBuffer.getError()) {
    llvm::errs() << "WARNING: could not read: " << compiledSourcePath
    << " error " << ec.message();
    return;
  }
  llvm::errs() << "HERE changed in " << primaryPath << "\n";
  auto changedRanges = diff(*errorOrPreviousBuffer->get(), *errorOrCurrentBuffer->get());

  auto inserted = rangesByPrimary.insert({primaryPath, changedRanges});
  assert(inserted.second && "Should not insert same primary twice.");
}

void ChangedSourceRangesForEachPrimary::dump() const {
  llvm::errs() << "\n*** changed source ranges: ***\n";
  llvm::yaml::Output dumper(llvm::errs());
  auto *mutableThis = const_cast<ChangedSourceRangesForEachPrimary*>(this);
  dumper << mutableThis->rangesByPrimary;
}

Ranges ChangedSourceRangesForEachPrimary::diff(const llvm::MemoryBuffer &old, const llvm::MemoryBuffer &nnew) {
  diff_match_patch<StringRef> differ;
  differ.Diff_Timeout = 0; // infinity
  differ.Diff_EditCost = 0;
  differ.Match_Threshold = 0; // perfection
  differ.Match_Distance = 1000000;
  auto comp = SourceComparator(old.getBuffer(), nnew.getBuffer());
  comp.compare();
  comp.dump();
  abort();
}
