//===------------- UnparsedRanges.cpp - Generates swiftdeps files ---------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

// TBD

#include "swift/AST/UnparsedRanges.h"
#include "swift/AST/Decl.h"
#include "swift/AST/DiagnosticEngine.h"
#include "swift/AST/DiagnosticsCommon.h"
#include "swift/AST/FileSystem.h"
#include "swift/Basic/SourceManager.h"
#include "swift/Parse/PersistentParserState.h"
#include "llvm/Support/YAMLParser.h"

using namespace swift;
using namespace unparsed_ranges;

SerializableSourceLocation::SerializableSourceLocation(
    const SourceLoc loc, const SourceManager &SM) {
  auto lc = SM.getLineAndColumn(loc);
  line = lc.first;
  column = lc.second;
}

SerializableSourceRange::SerializableSourceRange(const SourceRange r,
                                                 const SourceManager &SM)
    : start(SerializableSourceLocation(r.Start, SM)),
      end(SerializableSourceLocation(r.End, SM)) {}

void Emitter::emit() const {
  withOutputFile(diags, outputPath, [&](llvm::raw_pwrite_stream &out) {
    out << unparsedRangesFileHeader;
    emitRanges(out);
    return false;
  });
}

void Emitter::emitRanges(llvm::raw_ostream &out) const {
  llvm::yaml::Output yamlWriter(out);
  auto rangesByNonprimary = collectRanges();
  std::map<std::string, std::vector<SerializableSourceRange>>
      serializedRangesByNonprimary;
  for (auto &pair : rangesByNonprimary)
    serializedRangesByNonprimary.insert(
        {pair.first, serializeRanges(coalesceSortedRanges(
                         sortRanges(std::move(pair.second))))});
  yamlWriter << serializedRangesByNonprimary;
}

std::map<std::string, std::vector<SourceRange>> Emitter::collectRanges() const {
  std::map<std::string, std::vector<SourceRange>> rangesByNonprimaryFile;
  persistentState.forEachDelayedSourceRange(
      primaryFile, [&](const SourceRange sr) {
        const auto filename = sourceMgr.getIdentifierForBuffer(
            sourceMgr.findBufferContainingLoc(sr.Start));
        rangesByNonprimaryFile[filename].push_back(sr);
      });
  return rangesByNonprimaryFile;
}

std::vector<SourceRange>
Emitter::sortRanges(std::vector<SourceRange> ranges) const {
  std::sort(ranges.begin(), ranges.end(),
            [&](const SourceRange &lhs, const SourceRange &rhs) {
              return sourceMgr.isBeforeInBuffer(lhs.Start, rhs.Start);
            });
  return ranges;
}

std::vector<SourceRange>
Emitter::coalesceSortedRanges(std::vector<SourceRange> ranges) const {
  auto toBeWidened = ranges.begin();
  auto candidate = toBeWidened + 1;
  while (candidate < ranges.end()) {
    if (isImmediatelyBeforeOrOverlapping(*toBeWidened, *candidate))
      toBeWidened->widen(*candidate++);
    else if (++toBeWidened == candidate)
      ++candidate;
  }
  ranges.erase(candidate, ranges.end());
  return ranges;
}

std::vector<SerializableSourceRange>
Emitter::serializeRanges(std::vector<SourceRange> ranges) const {
  std::vector<SerializableSourceRange> result;
  for (const auto r : ranges)
    result.push_back(SerializableSourceRange(r, sourceMgr));
  return result;
}

bool Emitter::isImmediatelyBeforeOrOverlapping(SourceRange prev,
                                               SourceRange next) const {
  // TODO: investigate returning true if only white space intervenes.
  // Would be more work here, but less work downstream.
  return !sourceMgr.isBeforeInBuffer(prev.End, next.Start);
}
