// This test verifies that copies of dependency files are preserved after a
// compilation. For example, if the first compilation produces 'foo.swiftdeps',
// a second compilation should move 'foo.swiftdeps' to 'foo.swiftdeps~', then
// overwrite 'foo.swiftdeps' with new dependency information.

// RUN: %empty-directory(%t)

// First, produce the dependency files and verify their contents.
// RUN: %target-swift-frontend -enable-fine-grained-dependencies -emit-reference-dependencies-path %t.swiftdeps -typecheck -primary-file "%S/../Inputs/empty file.swift"
// RUN: awk '/kind:/ {k = $2}; /aspect:/ {a = $2}; /context:/ {c = $2}; /name/ {n = $2}; /sequenceNumber/ {s = $2}; /isProvides:/ {print k, a, c, n, $2}' <%t.swiftdeps   | sort  >%t-processed.swiftdeps
// RUN: %FileCheck -check-prefix=CHECK %s < %t-processed.swiftdeps

// CHECK-NOT: topLevel{{.*}}EmptyStruct{{.*}}true

// Next, produce the dependency files again, but this time using a different
// Swift source file than before. .swiftdeps~ should contain the same content
// as before. .swiftdeps should contain content that matches the new source
// file.
// RUN: %target-swift-frontend -enable-fine-grained-dependencies -emit-reference-dependencies-path %t.swiftdeps -typecheck -primary-file %S/../Inputs/global_resilience.swift
// RUN: %FileCheck -check-prefix=CHECK %s < %t.swiftdeps~
// RUN: awk '/kind:/ {k = $2}; /aspect:/ {a = $2}; /context:/ {c = $2}; /name/ {n = $2}; /sequenceNumber/ {s = $2}; /isProvides:/ {print k, a, c, n, $2}' <%t.swiftdeps   | sort  >%t-processed.swiftdeps
// RUN: %FileCheck -check-prefix=CHECK-OVERWRITTEN %s < %t-processed.swiftdeps

// CHECK-OVERWRITTEN:topLevel{{.*}}EmptyStruct{{.*}}true

