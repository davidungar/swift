//===------------- UnparsedRanges.h -----------------------------*- C++ -*-===//
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

#ifndef SWIFT_AST_UNPARSED_RANGES_H
#define SWIFT_AST_UNPARSED_RANGES_H

// Summary: TBD

#include "swift/Basic/LLVM.h"
#include "swift/Basic/SourceLoc.h"
#include "swift/Basic/StringExtras.h"
#include "llvm/Support/YAMLTraits.h"
#include <vector>

namespace swift {
class PersistentParserState;
class SourceManager;
class DiagnosticEngine;
class SourceFile;

namespace unparsed_ranges {

struct SerializableSourceLocation {
  uint64_t line = 0;
  uint64_t column = 0;

  SerializableSourceLocation(const SourceLoc loc, const SourceManager &SM);
  SerializableSourceLocation() = default;
};
struct SerializableSourceRange {
  SerializableSourceLocation start, end;

  SerializableSourceRange(const SourceRange r, const SourceManager &SM);
  SerializableSourceRange() = default;
};

typedef std::vector<SerializableSourceRange> Ranges;
typedef std::map<std::string, Ranges> RangesByNonprimary;
typedef std::map<std::string, RangesByNonprimary> RangesByNonprimaryByPrimary;

static constexpr const char *unparsedRangesFileHeader =
    "### Unparsed source ranges file v0 ###\n";

} // namespace unparsed_ranges
} // namespace swift

template <>
struct llvm::yaml::MappingTraits<
    swift::unparsed_ranges::SerializableSourceLocation> {
  static const bool flow = true;
  static void mapping(llvm::yaml::IO &io,
                      swift::unparsed_ranges::SerializableSourceLocation &loc) {
    io.mapRequired("line", loc.line), io.mapRequired("column", loc.column);
  }
};

template <>
struct llvm::yaml::MappingTraits<
    swift::unparsed_ranges::SerializableSourceRange> {
  static const bool flow = true;
  static void mapping(llvm::yaml::IO &io,
                      swift::unparsed_ranges::SerializableSourceRange &sr) {
    io.mapRequired("start", sr.start), io.mapRequired("end", sr.end);
  }
};

LLVM_YAML_IS_SEQUENCE_VECTOR(swift::unparsed_ranges::SerializableSourceRange)
LLVM_YAML_IS_STRING_MAP(swift::unparsed_ranges::Ranges)
LLVM_YAML_IS_STRING_MAP(swift::unparsed_ranges::RangesByNonprimary)

namespace swift {
namespace unparsed_ranges {
//==============================================================================
// MARK: Emitter
//==============================================================================

class Emitter {
  const StringRef outputPath;
  const SourceFile *const primaryFile;
  const PersistentParserState &persistentState;
  const SourceManager &sourceMgr;
  DiagnosticEngine &diags;

public:
  Emitter(StringRef outputPath, const SourceFile *primaryFile,
          const PersistentParserState &persistentState,
          const SourceManager &sourceMgr, DiagnosticEngine &diags)
      : outputPath(outputPath), primaryFile(primaryFile),
        persistentState(persistentState), sourceMgr(sourceMgr), diags(diags) {}

  void emit() const;

public:
  void emitRanges(llvm::raw_ostream &out) const;


private:
  std::map<std::string, std::vector<SourceRange>> collectRanges() const;
  std::vector<SourceRange> sortRanges(std::vector<SourceRange> ranges) const;

  /// Assuming \p ranges is sorted, coalesce overlapping ranges in place and
  /// return end of the resultant vector.
  std::vector<SourceRange> coalesceSortedRanges(std::vector<SourceRange>) const;

  std::vector<SerializableSourceRange>
  serializeRanges(std::vector<SourceRange> ranges) const;

  bool isImmediatelyBeforeOrOverlapping(SourceRange prev,
                                        SourceRange next) const;
};

} // namespace unparsed_ranges
} // namespace swift

#endif // SWIFT_AST_UNPARSED_RANGES_H
