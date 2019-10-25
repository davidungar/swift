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

#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"

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
  #error unimp
}

void ChangedSourceRangesForEachPrimary::dump() const {
  llvm::errs() << "\n*** changed source ranges: ***\n";
  llvm::yaml::Output dumper(llvm::errs());
  auto *mutableThis = const_cast<ChangedSourceRangesForEachPrimary*>(this);
  dumper << mutableThis->rangesByPrimary;
}
