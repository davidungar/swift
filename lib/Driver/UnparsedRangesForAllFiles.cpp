//===-- UnparsedRangesForAllFiles.cpp ------------------------------------==//
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

#include "swift/Driver/UnparsedRangesForAllFiles.h"

#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"

using namespace swift;
using namespace unparsed_ranges;

void UnparsedRangesForAllFiles::addRangesFromPath(
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

bool UnparsedRangesForAllFiles::addRangesFromBuffer(
    const StringRef primaryPath, const llvm::MemoryBuffer &buffer) {
  RangesByNonprimary rangesByNonprimaryFile;
  if (!buffer.getBuffer().startswith(unparsedRangesFileHeader))
    return true;

  llvm::yaml::Input yamlReader(llvm::MemoryBufferRef(buffer), nullptr);
  yamlReader >> rangesByNonprimaryFile;
  if (yamlReader.error())
    return true;
  addNonprimaryRangesForPrimary(primaryPath, std::move(rangesByNonprimaryFile));
  return false;
}

void UnparsedRangesForAllFiles::addNonprimaryRangesForPrimary(
    const StringRef primaryPath, RangesByNonprimary rangesByNonprimaryFile) {
  const auto inserted = rangesByNonprimaryByPrimary.insert(
      {primaryPath.str(), std::move(rangesByNonprimaryFile)});
  assert(inserted.second && "Should not have duplicate primaries.");
}

void UnparsedRangesForAllFiles::dump() {
  llvm::yaml::Output dumper(llvm::errs());
  dumper << rangesByNonprimaryByPrimary;
}
