//===--- UnparsedRangesForAllFiles.h ------------------*- C++-*-===//
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

#ifndef SWIFT_DRIVER_UNPARSEDRANGESFORALLFILES_H
#define SWIFT_DRIVER_UNPARSEDRANGESFORALLFILES_H

#include "swift/AST/UnparsedRanges.h"
#include "llvm/Support/MemoryBuffer.h"

namespace swift {
namespace unparsed_ranges {
class UnparsedRangesForAllFiles {
  RangesByNonprimaryByPrimary rangesByNonprimaryByPrimary;

public:
  void addRangesFromPath(StringRef primaryPath, StringRef unparsedRangesPath,
                         DiagnosticEngine &);

  void dump();

private:
  /// Return true if error
  bool addRangesFromBuffer(const StringRef primaryPath,
                           const llvm::MemoryBuffer &buffer);

  void addNonprimaryRangesForPrimary(const StringRef primaryPath,
                                     RangesByNonprimary rangesByNonprimaryFile);
};

} // namespace unparsed_ranges
} // namespace swift

#endif /* SWIFT_DRIVER_UNPARSEDRANGESFORALLFILES_H */
