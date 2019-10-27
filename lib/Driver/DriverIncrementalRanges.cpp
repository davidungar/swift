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
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <algorithm>

using namespace swift;
using namespace incremental_ranges;

//==============================================================================
// MARK: diff
//==============================================================================

class Comparator {
  typedef std::vector<StringRef> Collection;

  const Collection coll1, coll2;

  size_t start1 = 0, start2 = 0;
  size_t end1, end2;

  std::vector<Optional<size_t>> matches;

  struct PRLink {
    size_t coll1Index, coll2Index;
    PRLink* next;
  };

  std::vector<PRLink> linkVec;
  size_t nextLink = 0;

public:
  Comparator(StringRef s1, StringRef s2) :
  coll1(splitIntoLines(s1)), coll2(splitIntoLines(s2)),
  end1(coll1.size() - 1), end2(coll2.size() - 1),
  matches(coll1.size(), 0),
  linkVec(std::min(end1 - start1 + 1, end2 - start2 + 1), PRLink()) {
    computeMatchVector();
  }


  void dump() const {
    // for every matching line in coll1
    for (size_t possibleColl1Match = 0;
         possibleColl1Match < matches.size();
        ) {
      // advance to last line of coll1 in match group
      auto nextLinesAfterThisMatchGroup = getNextLinesAfterThisMatchGroup(possibleColl1Match);
      auto lastUnmatchedLines = getLastUnmatchedLinesStartingAt(nextLinesAfterThisMatchGroup.first - 1);

      reportChange( nextLinesAfterThisMatchGroup.first,  lastUnmatchedLines.first,
                    nextLinesAfterThisMatchGroup.second, lastUnmatchedLines.second);
      possibleColl1Match = lastUnmatchedLines.first + 1;
    }
  }

private:
  std::pair<size_t, size_t> getNextLinesAfterThisMatchGroup(const size_t coll1Index) const {
    auto getNextColl2Line = [&](size_t nextColl1Line) {
      return nextColl1Line == 0 ? 0 :  matches[nextColl1Line - 1].getValue() + 1;
    };
    size_t nextColl1Line = coll1Index,
          nextColl2Line = getNextColl2Line(coll1Index);
    for (  ;
         nextColl1Line < matches.size() &&
         matches[nextColl1Line] &&
         matches[nextColl1Line].getValue() == nextColl2Line;
         ++nextColl1Line, nextColl2Line = getNextColl2Line(nextColl1Line) ) {
    }
    return {nextColl1Line, nextColl2Line};
  }
  std::pair<size_t, size_t>getLastUnmatchedLinesStartingAt(const size_t lastColl1LineInMatchGroup) const {
    auto lastUnmatchedColl1Line = lastColl1LineInMatchGroup;
    for ( ; lastUnmatchedColl1Line < matches.size() - 1  &&  !matches[lastUnmatchedColl1Line + 1]; ++lastUnmatchedColl1Line) {}
    auto lastUnmatchedColl2Line = lastUnmatchedColl1Line < matches.size()
      ? matches[lastUnmatchedColl1Line + 1].getValue() - 1 : coll2.size() - 1;
    return {lastUnmatchedColl1Line, lastUnmatchedColl2Line};
  }
  void reportChange(const size_t coll1Start, const size_t coll1End,
                    const size_t coll2Start, const size_t coll2End) const {
    auto &out = llvm::errs();
    auto printRange = [&](size_t rangeStart, size_t rangeEnd) {
      // 1-origin for llvm compat
      ++rangeStart, ++rangeEnd;
      out << std::min(rangeStart, rangeEnd);
      if (rangeStart < rangeEnd)
        out << ", " << rangeEnd;
    };

    if (coll1Start > coll1End && coll2Start > coll2End)
      return;
    printRange(coll1Start, coll1End);
    {
      const auto added = "a", deleted = "d", changed = "c";
      out << (coll1Start > coll1End ? added : coll2Start > coll2End ? deleted : changed);
    }
    printRange(coll2Start, coll2End);
    out << "\n";

    for (auto i = coll1Start; i <= coll1End; ++i)
      out << "< " << coll1[i] << "\n";
    if (coll1Start <= coll1End && coll2Start <= coll2End)
      out << "---\n";
    for (auto i = coll2Start; i <= coll2End; ++i)
      out << "> " << coll2[i] << "\n";
  }

  static std::vector<StringRef> splitIntoLines(StringRef s) {
    std::vector<StringRef> result;
    for (;;) {
      auto lineRest = s.split('\n');
      result.push_back(lineRest.first);
      if (lineRest.second.empty())
        return result;
    }
  }

  void computeMatchVector() {
    // Trim ends of common elements
    trimStart(), trimEnd();

    scan(buildDAGOfSubsequences(buildEquivalenceClasses()));
  }

  void trimStart() {
    for ( ;
         start1 <= end1 && start2 <= end2 && coll1[start1] == coll2[start2];
         ++start1, ++start2)
      matches[start1 - 0] = start2;
  }
  void trimEnd() {
    for ( ;
         start1 <= end1 && start2 <= end2 && coll1[end1] == coll2[end2];
         --end1, --end2)
      matches[end1 - 0] = end2;
  }

  /// Return a map from a line in coll2 to indices of all identical lines in coll2
  std::unordered_map<std::string, std::vector<size_t>>
  buildEquivalenceClasses() {
    // Map each element of coll2 to the sequence of indices it occupies.
    std::unordered_map<std::string, std::vector<size_t>> coll2Map;
    for (auto index = start2; index <= end2; ++index)
      coll2Map[coll2[index]].push_back(index);
    return coll2Map;
  }



  class SortedSequence {
    typedef size_t Elem;
    std::vector<Elem> contents;
  public:
    Optional<Elem> replaceNextLargerWith(const Elem x) {
      // Find the place at which x would normally be inserted into the sequence,
      // and replace that element with x, returning the index.
      // If x is already in the sequence, do nothing and return None.
      // Preserve sort order.
      auto lastEl = contents.back();
      if (lastEl < x) {
        contents.push_back(x);
        return contents.size() - 1;
      }
      if (auto index = indexOfFirstGreaterElement(x)) {
        contents[*index] = x;
        return index;
      }
      contents.push_back(x);
      return contents.size() - 1;
    }

    bool empty() const { return contents.empty(); }

    Elem back() const { return contents.back();}
  private:
    Optional<size_t> indexOfFirstGreaterElement(const Elem x) {
      // return index of first greater element
#warning redo
      assert(!contents.empty());
      for (size_t i = 0; i < contents.size(); ++i)
        if (x < contents[i])
          return i;
      return None;
    }
  };

  PRLink *newPRLink(size_t coll1ndex, size_t coll2Index, PRLink* next) {
    assert(nextLink < linkVec.size());
    linkVec[nextLink].coll1Index = coll1ndex;
    linkVec[nextLink].coll2Index = coll2Index;
    linkVec[nextLink].next = next;
    return &linkVec[nextLink++];
  }

  std::pair<std::vector<PRLink*>, SortedSequence>
  buildDAGOfSubsequences(std::unordered_map<std::string, std::vector<size_t>> coll2Map) {
    SortedSequence thresh;
    std::vector<PRLink*> links(std::min(end1 - start1 + 1, end2 - start2 + 1), nullptr);
    for (auto i = start1; i <= end1; ++i) {
      // What lines in coll2 does the ith line in coll1 match?
      auto iter = coll2Map.find(coll1[i]);
      if (iter == coll2Map.end())
        continue; // no match in coll2
      auto &res = iter->second;
      size_t lastJ = ~0;
      // For each match in coll2 in reverse order
      for (auto j: llvm::reverse(res)) {
        assert(j < lastJ); lastJ = j;
        // replace the index of the match with the index of the earlier match
        // or add it if last match
        // (thresh gets populated for each coll1 match with coll2 indices)
        if (auto optK = thresh.replaceNextLargerWith(j)) {
          // There was a later match at thresh[k]
          auto k = optK.getValue();
          // prev match (of what?) was at links[k-1]?
          // chain to that match
          PRLink* newNext = k == 0 ? nullptr : links[k-1];
          links[k] = newPRLink(i, j, newNext);
        }
      }
    }
    return {links, thresh};
  }

  void scan(std::pair<std::vector<PRLink*>, SortedSequence> linksAndThres) {
    auto links = linksAndThres.first;
    auto thresh = linksAndThres.second;

    // For every match put coll2 index in matches[col2 index]
    for (auto lnk = thresh.empty() ? nullptr : links[thresh.back()];
         lnk;
         lnk = lnk->next)
      matches[lnk->coll1Index] = lnk->coll2Index;
  }

};

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
  auto comp = Comparator(old.getBuffer(), nnew.getBuffer());
  comp.dump();
  abort();
}
