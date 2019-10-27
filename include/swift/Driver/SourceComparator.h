//===------------------------------ SourceComparator.h -----------*- C++-*-===//
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

#ifndef SWIFT_DRIVER_SOURCECOMPARATOR_H
#define SWIFT_DRIVER_SOURCECOMPARATOR_H

#include "swift/AST/IncrementalRanges.h"
#include "llvm/Support/MemoryBuffer.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace swift {
namespace incremental_ranges {

class SourceComparator {
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



  //==============================================================================
    // MARK: SortedSequence
    //==============================================================================
  private:

  class SortedSequence {
    typedef size_t Elem;
    std::vector<Elem> contents;
  public:
    /// Find the place at which x would normally be inserted into the sequence,
     /// and replace that element with x, returning the index.
     /// If x is already in the sequence, do nothing and return None.
     /// Preserve sort order.
    Optional<Elem> replaceNextLargerWith(const Elem x);
    bool empty() const { return contents.empty(); }

    Elem back() const { return contents.back();}

  private:
    /// return index of first greater element
    Optional<size_t> indexOfFirstGreaterElement(const Elem x) const;
  };


  //==============================================================================
   // MARK: initialization
   //==============================================================================

public:
  SourceComparator(StringRef s1, StringRef s2);

private:
  static std::vector<StringRef> splitIntoLines(StringRef s);

   //==============================================================================
    // MARK: Comparing
    //==============================================================================
  public:

    void compare();

  private:
  void trimStart();
  void trimEnd();

  /// Return a map from a line in coll2 to indices of all identical lines in coll2
  std::unordered_map<std::string, std::vector<size_t>>
  buildEquivalenceClasses();

  PRLink *newPRLink(size_t coll1ndex, size_t coll2Index, PRLink* next);


  std::pair<std::vector<PRLink*>, SortedSequence>
  buildDAGOfSubsequences(std::unordered_map<std::string, std::vector<size_t>> coll2Map);

  void scanMatchedLines(std::pair<std::vector<PRLink*>, SortedSequence> linksAndThres); 

  //==============================================================================
  // MARK: printing
  //==============================================================================
public:

  void dump() const { print(llvm::errs()); }
  void print(raw_ostream&) const;

private:
  std::pair<size_t, size_t> getNextLinesAfterThisMatchGroup(const size_t coll1Index) const;
  std::pair<size_t, size_t>getLastUnmatchedLinesStartingAt(const size_t lastColl1LineInMatchGroup) const;

  void reportChange(const size_t coll1Start, const size_t coll1End,
                    const size_t coll2Start, const size_t coll2End,
                    raw_ostream &) const;




};

} // namespace incremental_ranges
} // namespace swift

#endif /* SWIFT_DRIVER_SOURCECOMPARATOR_H */
