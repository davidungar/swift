typealias A = B & protocol<C, D>

// RUN: not %target-swift-frontend -typecheck %s -dump-ast |  %FileCheck %s
// CHECK:   (typealias range=[SOURCE_DIR/test/Parse/type_alias_range.swift:1:1 - line:1:32] {{.*}}
