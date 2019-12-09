#include "swift/Basic/ReferenceDependencyKeys.h"
#include "swift/Driver/CoarseGrainedDependencyGraph.h"
#include "swift/Driver/FineGrainedDependencyDriverGraph.h"
#include "swift/Driver/Job.h"
#include "gtest/gtest.h"

using namespace swift;
using LoadResult = CoarseGrainedDependencyGraphImpl::LoadResult;
using namespace reference_dependency_keys;
using namespace fine_grained_dependencies;
using Job = driver::Job;

static LoadResult loadFromString(ModuleDepGraph &dg, const Job *node,
                                 StringRef key, StringRef data) {
  return dg.loadFromString(node, key.str() + ": [" + data.str() + "]");
}

static LoadResult loadFromString(ModuleDepGraph &dg, const Job *node,
                                 StringRef key1, StringRef data1,
                                 StringRef key2, StringRef data2) {
  return dg.loadFromString(node, key1.str() + ": [" + data1.str() + "]\n" +
                                     key2.str() + ": [" + data2.str() + "]");
}

static LoadResult loadFromString(ModuleDepGraph &dg, const Job *node,
                                 StringRef key1, StringRef data1,
                                 StringRef key2, StringRef data2,
                                 StringRef key3, StringRef data3,
                                 StringRef key4, StringRef data4) {
  return dg.loadFromString(node, key1.str() + ": [" + data1.str() + "]\n" +
                                     key2.str() + ": [" + data2.str() + "]\n" +
                                     key3.str() + ": [" + data3.str() + "]\n" +
                                     key4.str() + ": [" + data4.str() + "]\n");
}

TEST(ModuleDepGraph, BasicLoad) {
  ModuleDepGraph graph;
  Job j0, j1, j2, j3, j4, j5, j6, j7, j8, j9;

  EXPECT_EQ(loadFromString(graph, &j0, dependsTopLevel, "a, b"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "c, d"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, providesTopLevel, "e, f"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j3, providesNominal, "g, h"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j4, providesDynamicLookup, "i, j"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j5, dependsDynamicLookup, "k, l"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j6, providesMember, "[m, mm], [n, nn]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j7, dependsMember, "[o, oo], [p, pp]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j8, dependsExternal, "/foo, /bar"),
            LoadResult::UpToDate);

  EXPECT_EQ(loadFromString(graph, &j9, providesNominal, "a, b",
                           providesTopLevel, "b, c", dependsNominal, "c, d",
                           dependsTopLevel, "d, a"),
            LoadResult::UpToDate);
}

TEST(ModuleDepGraph, IndependentNodes) {
  ModuleDepGraph graph;
  Job j0, j1, j2;

  EXPECT_EQ(
      loadFromString(graph, &j0, dependsTopLevel, "a", providesTopLevel, "a0"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsTopLevel, "b", providesTopLevel, "b0"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j2, dependsTopLevel, "c", providesTopLevel, "c0"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));

  // Mark 0 again -- should be no change.
  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));

  EXPECT_EQ(0u, graph.markTransitive(&j2).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));

  EXPECT_EQ(0u, graph.markTransitive(&j1).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, IndependentDepKinds) {
  ModuleDepGraph graph;
  Job j0, j1;

  EXPECT_EQ(
      loadFromString(graph, &j0, dependsNominal, "a", providesNominal, "b"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsTopLevel, "b", providesTopLevel, "a"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, IndependentDepKinds2) {
  ModuleDepGraph graph;
  Job j0, j1;

  EXPECT_EQ(
      loadFromString(graph, &j0, dependsNominal, "a", providesNominal, "b"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsTopLevel, "b", providesTopLevel, "a"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&j1).size());
  EXPECT_FALSE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, IndependentMembers) {
  ModuleDepGraph graph;
  Job j0, j1, j2, j3, j4;

  EXPECT_EQ(loadFromString(graph, &j0, providesMember, "[a,aa]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsMember, "[a,bb]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsMember, "[a,\"\"]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j3, dependsMember, "[b,aa]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j4, dependsMember, "[b,bb]"),
            LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));
  EXPECT_FALSE(graph.isMarked(&j3));
  EXPECT_FALSE(graph.isMarked(&j4));
}

TEST(ModuleDepGraph, SimpleDependent) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);
  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependentReverse) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, dependsTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, providesTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j1);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j0, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependent2) {
  ModuleDepGraph graph;
  Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "x, b, z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependent3) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(
      loadFromString(graph, &j0, providesNominal, "a", providesTopLevel, "a"),
      LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "a"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependent4) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsNominal, "a", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependent5) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(
      loadFromString(graph, &j0, providesNominal, "a", providesTopLevel, "a"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsNominal, "a", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  auto marked = graph.markTransitive(&j0);
  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependent6) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesDynamicLookup, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsDynamicLookup, "x, b, z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleDependentMember) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(
      loadFromString(graph, &j0, providesMember, "[a,aa], [b,bb], [c,cc]"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsMember, "[x, xx], [b,bb], [z,zz]"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

template <typename Range, typename T>
static bool contains(const Range &range, const T &value) {
  return std::find(std::begin(range), std::end(range), value) !=
         std::end(range);
}

TEST(ModuleDepGraph, MultipleDependentsSame) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "x, b, z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "q, b, s"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&j0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &j1));
    EXPECT_TRUE(contains(marked, &j2));
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));

  EXPECT_EQ(0u, graph.markTransitive(&j0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, MultipleDependentsDifferent) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "x, b, z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "q, r, c"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &j1));
    EXPECT_TRUE(contains(marked, &j2));
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, ChainedDependents) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsNominal, "x, b", providesNominal, "z"),
      LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &j1));
    EXPECT_TRUE(contains(marked, &j2));
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, MarkTwoNodes) {
  ModuleDepGraph graph;
  const Job j0, j1, j2, j10, j11, j12;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a, b"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsNominal, "a", providesNominal, "z"),
      LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "z"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j10, providesNominal, "y, z", dependsNominal, "q"),
      LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j11, dependsNominal, "y"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j12, dependsNominal, "q", providesNominal, "q"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &j1));
    EXPECT_TRUE(contains(marked, &j2));
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
  EXPECT_FALSE(graph.isMarked(&j10));
  EXPECT_FALSE(graph.isMarked(&j11));
  EXPECT_FALSE(graph.isMarked(&j12));

  {
    auto marked = graph.markTransitive(&j10);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j11, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
  EXPECT_TRUE(graph.isMarked(&j10));
  EXPECT_TRUE(graph.isMarked(&j11));
  EXPECT_FALSE(graph.isMarked(&j12));
}

TEST(ModuleDepGraph, MarkOneNodeTwice) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));

  // Reload 0.
  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j2, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, MarkOneNodeTwice2) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));

  // Reload 0.
  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a, b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j2, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, NotTransitiveOnceMarked) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsNominal, "b"),
            LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&j1).size());
  EXPECT_FALSE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));

  // Reload 1.
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsNominal, "a", providesNominal, "b"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_FALSE(graph.isMarked(&j2));

  // Re-mark 1.
  {
    auto marked = graph.markTransitive(&j1);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j2, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, DependencyLoops) {
  ModuleDepGraph graph;
  const Job j0, j1, j2;

  EXPECT_EQ(loadFromString(graph, &j0, providesTopLevel, "a, b, c",
                           dependsTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, providesTopLevel, "x", dependsTopLevel,
                           "x, b, z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j2, dependsTopLevel, "x"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &j1));
    EXPECT_TRUE(contains(marked, &j2));
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
  EXPECT_TRUE(graph.isMarked(&j2));
}

TEST(ModuleDepGraph, MarkIntransitive) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  EXPECT_TRUE(graph.markIntransitive(0));
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, MarkIntransitiveTwice) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  EXPECT_TRUE(graph.markIntransitive(0));
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));

  EXPECT_FALSE(graph.markIntransitive(0));
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, MarkIntransitiveThenIndirect) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &j1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  EXPECT_TRUE(graph.markIntransitive(&j1));
  EXPECT_FALSE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, SimpleExternal) {
  ModuleDepGraph graph;
  const Job j0;

  EXPECT_EQ(loadFromString(graph, &j0, dependsExternal, "/foo, /bar"),
            LoadResult::UpToDate);

  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/foo"));
  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/bar"));

  EXPECT_EQ(1u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&j0));

  EXPECT_EQ(0u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&j0));
}

TEST(ModuleDepGraph, SimpleExternal2) {
  ModuleDepGraph graph;
  const Job j0;

  EXPECT_EQ(loadFromString(graph, &j0, dependsExternal, "/foo, /bar"),
            LoadResult::UpToDate);

  EXPECT_EQ(1u, graph.markExternal("/bar").size());
  EXPECT_TRUE(graph.isMarked(&j0));

  EXPECT_EQ(0u, graph.markExternal("/bar").size());
  EXPECT_TRUE(graph.isMarked(&j0));
}

TEST(ModuleDepGraph, ChainedExternal) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, dependsExternal, "/foo",
                           providesTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsExternal, "/bar", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/foo"));
  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/bar"));

  EXPECT_EQ(2u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, ChainedExternalReverse) {
  ModuleDepGraph graph;
  const Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, dependsExternal, "/foo",
                           providesTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsExternal, "/bar", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markExternal("/bar");
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&j1, marked.front());
  }
  EXPECT_FALSE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  EXPECT_EQ(0u, graph.markExternal("/bar").size());
  EXPECT_FALSE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));

  {
    auto marked = graph.markExternal("/foo");
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(0u, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_TRUE(graph.isMarked(&j1));
}

TEST(ModuleDepGraph, ChainedExternalPreMarked) {
  ModuleDepGraph graph;
  Job j0, j1;

  EXPECT_EQ(loadFromString(graph, &j0, dependsExternal, "/foo",
                           providesTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &j1, dependsExternal, "/bar", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  graph.markIntransitive(0);

  EXPECT_EQ(0u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&j0));
  EXPECT_FALSE(graph.isMarked(&j1));
}
