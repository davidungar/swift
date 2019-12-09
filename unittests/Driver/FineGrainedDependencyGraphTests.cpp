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

static LoadResult loadFromString(ModuleDepGraph &dg, const Job *cmd,
                                 StringRef key, StringRef data) {
  return dg.loadFromString(cmd, key.str() + ": [" + data.str() + "]");
}

static LoadResult loadFromString(ModuleDepGraph &dg, const Job *cmd,
                                 StringRef key1, StringRef data1,
                                 StringRef key2, StringRef data2) {
  return dg.loadFromString(cmd, key1.str() + ": [" + data1.str() + "]\n" +
                                    key2.str() + ": [" + data2.str() + "]");
}

static LoadResult loadFromString(ModuleDepGraph &dg, const Job *cmd,
                                 StringRef key1, StringRef data1,
                                 StringRef key2, StringRef data2,
                                 StringRef key3, StringRef data3,
                                 StringRef key4, StringRef data4) {
  return dg.loadFromString(cmd, key1.str() + ": [" + data1.str() + "]\n" +
                                    key2.str() + ": [" + data2.str() + "]\n" +
                                    key3.str() + ": [" + data3.str() + "]\n" +
                                    key4.str() + ": [" + data4.str() + "]\n");
}

static LoadResult simulateLoad(ModuleDepGraph &dg, const Job *cmd,
                               StringRef key, std::vector<StringRef> names) {
  auto g = SourceFileDepGraph();
  return dg.loadFromString(cmd, key.str() + ": [" + data.str() + "]");
}

static OutputFileMap OFM;
static Job
  job0(OFM, "0"),
  job1(OFM, "1"),
  job2(OFM, "2"),
  job3(OFM, "3"),
  job4(OFM, "4"),
  job5(OFM, "5"),
  job6(OFM, "6"),
  job7(OFM, "7"),
  job8(OFM, "8"),
  job9(OFM, "9"),
  job10(OFM, "10"),
  job11(OFM, "11"),
  job12(OFM, "12");

TEST(ModuleDepGraph, BasicLoad) {
  ModuleDepGraph graph;

  EXPECT_EQ(simulateLoad(graph, &job0, dependsTopLevel, {"a", "b"}),
            LoadResult::UpToDate);
  EXPECT_EQ(simulateLoad(graph, &job1, dependsNominal, {"c", "d"}),
            LoadResult::UpToDate);
  EXPECT_EQ(simulateLoad(graph, &job2, providesTopLevel, {"e", "f"}),
            LoadResult::UpToDate);
  EXPECT_EQ(simulateLoad(graph, &job3, providesNominal, {"g", "h"}),
            LoadResult::UpToDate);
  EXPECT_EQ(simulateLoad(graph, &job4, providesDynamicLookup, {"i", "j"}),
            LoadResult::UpToDate);
  EXPECT_EQ(simulateLoad(graph, &job5, dependsDynamicLookup, {"k", "l"}),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job6, providesMember, "[m, mm], [n, nn]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job7, dependsMember, "[o, oo], [p, pp]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job8, dependsExternal, "/foo, /bar"),
            LoadResult::UpToDate);

  EXPECT_EQ(loadFromString(graph, &job9, providesNominal, "a, b",
                           providesTopLevel, "b, c", dependsNominal, "c, d",
                           dependsTopLevel, "d, a"),
            LoadResult::UpToDate);
}

TEST(ModuleDepGraph, IndependentNodes) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsTopLevel, "a", providesTopLevel,
                           "a0"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsTopLevel, "b", providesTopLevel,
                           "b0"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsTopLevel, "c", providesTopLevel,
                           "c0"),
            LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));

  // Mark 0 again -- should be no change.
  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));

  EXPECT_EQ(0u, graph.markTransitive(&job2).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));

  EXPECT_EQ(0u, graph.markTransitive(&job1).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, IndependentDepKinds) {
  ModuleDepGraph graph;

  EXPECT_EQ(
      loadFromString(graph, &job0, dependsNominal, "a", providesNominal, "b"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsTopLevel, "b", providesTopLevel, "a"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, IndependentDepKinds2) {
  ModuleDepGraph graph;

  EXPECT_EQ(
      loadFromString(graph, &job0, dependsNominal, "a", providesNominal, "b"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsTopLevel, "b", providesTopLevel, "a"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&job1).size());
  EXPECT_FALSE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, IndependentMembers) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesMember, "[a,aa]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsMember, "[a,bb]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsMember, "[a,\"\"]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job3, dependsMember, "[b,aa]"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job4, dependsMember, "[b,bb]"),
            LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));
  EXPECT_FALSE(graph.isMarked(&job3));
  EXPECT_FALSE(graph.isMarked(&job4));
}

TEST(ModuleDepGraph, SimpleDependent) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);
  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependentReverse) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, providesTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job1);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job0, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependent2) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "x, b, z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependent3) {
  ModuleDepGraph graph;

  EXPECT_EQ(
      loadFromString(graph, &job0, providesNominal, "a", providesTopLevel, "a"),
      LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "a"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependent4) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsNominal, "a", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependent5) {
  ModuleDepGraph graph;

  EXPECT_EQ(
      loadFromString(graph, &job0, providesNominal, "a", providesTopLevel, "a"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsNominal, "a", dependsTopLevel, "a"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  auto marked = graph.markTransitive(&job0);
  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependent6) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesDynamicLookup, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsDynamicLookup, "x, b, z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleDependentMember) {
  ModuleDepGraph graph;

  EXPECT_EQ(
      loadFromString(graph, &job0, providesMember, "[a,aa], [b,bb], [c,cc]"),
      LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsMember, "[x, xx], [b,bb], [z,zz]"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

template <typename Range, typename T>
static bool contains(const Range &range, const T &value) {
  return std::find(std::begin(range), std::end(range), value) !=
         std::end(range);
}

TEST(ModuleDepGraph, MultipleDependentsSame) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "x, b, z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "q, b, s"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(&job0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &job1));
    EXPECT_TRUE(contains(marked, &job2));
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));

  EXPECT_EQ(0u, graph.markTransitive(&job0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, MultipleDependentsDifferent) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "x, b, z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "q, r, c"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &job1));
    EXPECT_TRUE(contains(marked, &job2));
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, ChainedDependents) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "x, b",
                           providesNominal, "z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "z"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &job1));
    EXPECT_TRUE(contains(marked, &job2));
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, MarkTwoNodes) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a, b"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsNominal, "a", providesNominal, "z"),
      LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job10, providesNominal, "y, z",
                           dependsNominal, "q"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job11, dependsNominal, "y"),
            LoadResult::UpToDate);
  EXPECT_EQ(
      loadFromString(graph, &job12, dependsNominal, "q", providesNominal, "q"),
      LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &job1));
    EXPECT_TRUE(contains(marked, &job2));
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
  EXPECT_FALSE(graph.isMarked(&job10));
  EXPECT_FALSE(graph.isMarked(&job11));
  EXPECT_FALSE(graph.isMarked(&job12));

  {
    auto marked = graph.markTransitive(&job10);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job11, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
  EXPECT_TRUE(graph.isMarked(&job10));
  EXPECT_TRUE(graph.isMarked(&job11));
  EXPECT_FALSE(graph.isMarked(&job12));
}

TEST(ModuleDepGraph, MarkOneNodeTwice) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));

  // Reload 0.
  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job2, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, MarkOneNodeTwice2) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));

  // Reload 0.
  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a, b"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job2, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, NotTransitiveOnceMarked) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsNominal, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsNominal, "b"),
            LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(&job1).size());
  EXPECT_FALSE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));

  // Reload 1.
  EXPECT_EQ(
      loadFromString(graph, &job1, dependsNominal, "a", providesNominal, "b"),
      LoadResult::UpToDate);

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_FALSE(graph.isMarked(&job2));

  // Re-mark 1.
  {
    auto marked = graph.markTransitive(&job1);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job2, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, DependencyLoops) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesTopLevel, "a, b, c",
                           dependsTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, providesTopLevel, "x", dependsTopLevel,
                           "x, b, z"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job2, dependsTopLevel, "x"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(2u, marked.size());
    EXPECT_TRUE(contains(marked, &job1));
    EXPECT_TRUE(contains(marked, &job2));
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
  EXPECT_TRUE(graph.isMarked(&job2));
}

TEST(ModuleDepGraph, MarkIntransitive) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  EXPECT_TRUE(graph.markIntransitive(0));
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));

  {
    auto marked = graph.markTransitive(0);
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, MarkIntransitiveTwice) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  EXPECT_TRUE(graph.markIntransitive(0));
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));

  EXPECT_FALSE(graph.markIntransitive(0));
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, MarkIntransitiveThenIndirect) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, providesTopLevel, "a, b, c"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsTopLevel, "x, b, z"),
            LoadResult::UpToDate);

  EXPECT_TRUE(graph.markIntransitive(&job1));
  EXPECT_FALSE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markTransitive(0).size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, SimpleExternal) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsExternal, "/foo, /bar"),
            LoadResult::UpToDate);

  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/foo"));
  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/bar"));

  EXPECT_EQ(1u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&job0));

  EXPECT_EQ(0u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&job0));
}

TEST(ModuleDepGraph, SimpleExternal2) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsExternal, "/foo, /bar"),
            LoadResult::UpToDate);

  EXPECT_EQ(1u, graph.markExternal("/bar").size());
  EXPECT_TRUE(graph.isMarked(&job0));

  EXPECT_EQ(0u, graph.markExternal("/bar").size());
  EXPECT_TRUE(graph.isMarked(&job0));
}

TEST(ModuleDepGraph, ChainedExternal) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsExternal, "/foo",
                           providesTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsExternal, "/bar",
                           dependsTopLevel, "a"),
            LoadResult::UpToDate);

  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/foo"));
  EXPECT_TRUE(contains(graph.getExternalDependencies(), "/bar"));

  EXPECT_EQ(2u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, ChainedExternalReverse) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsExternal, "/foo",
                           providesTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsExternal, "/bar",
                           dependsTopLevel, "a"),
            LoadResult::UpToDate);

  {
    auto marked = graph.markExternal("/bar");
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(&job1, marked.front());
  }
  EXPECT_FALSE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  EXPECT_EQ(0u, graph.markExternal("/bar").size());
  EXPECT_FALSE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));

  {
    auto marked = graph.markExternal("/foo");
    EXPECT_EQ(1u, marked.size());
    EXPECT_EQ(0u, marked.front());
  }
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_TRUE(graph.isMarked(&job1));
}

TEST(ModuleDepGraph, ChainedExternalPreMarked) {
  ModuleDepGraph graph;

  EXPECT_EQ(loadFromString(graph, &job0, dependsExternal, "/foo",
                           providesTopLevel, "a"),
            LoadResult::UpToDate);
  EXPECT_EQ(loadFromString(graph, &job1, dependsExternal, "/bar",
                           dependsTopLevel, "a"),
            LoadResult::UpToDate);

  graph.markIntransitive(0);

  EXPECT_EQ(0u, graph.markExternal("/foo").size());
  EXPECT_TRUE(graph.isMarked(&job0));
  EXPECT_FALSE(graph.isMarked(&job1));
}
