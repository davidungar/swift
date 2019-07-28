// Note: test of the scope map. All of these tests are line- and
// column-sensitive, so any additions should go at the end.

struct S0 { }

let a: Int? = 1

guard let b = a else {
}

func foo() {} // to interrupt the TopLevelCodeDecl

let c = b

typealias T = Int

extension Int {
  func my_identity() -> Int { return self }
}

var i: Int = b.my_identity()


// RUN: %target-swift-frontend -dump-scope-maps expanded %s 2> %t.expanded
// RUN: %FileCheck -check-prefix CHECK-EXPANDED %s < %t.expanded


// CHECK-EXPANDED:      ***Complete scope map***
// CHECK-EXPANDED-NEXT: ASTSourceFileScope 0x{{.*}}, (uncached) [1:1 - 6{{[0-9]}}:1] 'SOURCE_DIR/test/NameBinding/scope_map_top_level.swift'
// CHECK-EXPANDED-NEXT: |-NominalTypeDeclScope 0x{{.*}}, [4:1 - 4:13] 'S0'
// CHECK-EXPANDED-NEXT:   `-NominalTypeBodyScope 0x{{.*}}, [4:11 - 4:13] 'S0'
// CHECK-EXPANDED-NEXT: `-TopLevelCodeScope 0x{{.*}}, [6:1 - 21:28]
// CHECK-EXPANDED-NEXT:   `-BraceStmtScope 0x{{.*}}, [6:1 - 21:28]
// CHECK-EXPANDED-NEXT:     |-PatternEntryDeclScope 0x{{.*}}, [6:5 - 6:15] entry 0 'a'
// CHECK-EXPANDED-NEXT:       `-PatternEntryInitializerScope 0x{{.*}}, [6:15 - 6:15] entry 0 'a'
// CHECK-EXPANDED-NEXT:     `-TopLevelCodeScope 0x{{.*}}, [8:1 - 21:28]
// CHECK-EXPANDED-NEXT:       `-BraceStmtScope 0x{{.*}}, [8:1 - 21:28]
// CHECK-EXPANDED-NEXT:         `-GuardStmtScope 0x{{.*}}, [8:1 - 21:28]
// CHECK-EXPANDED-NEXT:           |-ConditionalClauseScope, [8:7 - 8:22] index 0
// CHECK-EXPANDED-NEXT:             `-ConditionalClausePatternUseScope, [8:22 - 8:22] let b?
// CHECK-EXPANDED-NEXT:           |-BraceStmtScope 0x{{.*}}, [8:22 - 9:1]
// CHECK-EXPANDED-NEXT:           `-LookupParentDiversionScope, [9:1 - 21:28]
// CHECK-EXPANDED-NEXT:             |-AbstractFunctionDeclScope 0x{{.*}}, [11:1 - 11:13] 'foo()'
// CHECK-EXPANDED-NEXT:               `-ParameterListScope 0x{{.*}}, [11:9 - 11:13]
// CHECK-EXPANDED-NEXT:                 `-PureFunctionBodyScope 0x{{.*}}, [11:12 - 11:13]
// CHECK-EXPANDED-NEXT:                   `-BraceStmtScope 0x{{.*}}, [11:12 - 11:13]
// CHECK-EXPANDED-NEXT:             `-TopLevelCodeScope 0x{{.*}}, [13:1 - 21:28]
// CHECK-EXPANDED-NEXT:               `-BraceStmtScope 0x{{.*}}, [13:1 - 21:28]
// CHECK-EXPANDED-NEXT:                 |-PatternEntryDeclScope 0x{{.*}}, [13:5 - 13:9] entry 0 'c'
// CHECK-EXPANDED-NEXT:                   `-PatternEntryInitializerScope 0x{{.*}}, [13:9 - 13:9] entry 0 'c'
// CHECK-EXPANDED-NEXT:                 |-TypeAliasDeclScope 0x{{.*}}, [15:1 - 15:15] <no extended nominal?!>
// CHECK-EXPANDED-NEXT:                 |-ExtensionDeclScope 0x{{.*}}, [17:1 - 19:1] 'Int'
// CHECK-EXPANDED-NEXT:                   `-ExtensionBodyScope 0x{{.*}}, [17:15 - 19:1] 'Int'
// CHECK-EXPANDED-NEXT:                     `-AbstractFunctionDeclScope 0x{{.*}}, [18:3 - 18:43] 'my_identity()'
// CHECK-EXPANDED-NEXT:                       `-ParameterListScope 0x{{.*}}, [18:19 - 18:43]
// CHECK-EXPANDED-NEXT:                         `-MethodBodyScope 0x{{.*}}, [18:29 - 18:43]
// CHECK-EXPANDED-NEXT:                           `-BraceStmtScope 0x{{.*}}, [18:29 - 18:43]
// CHECK-EXPANDED-NEXT:                 `-TopLevelCodeScope 0x{{.*}}, [21:1 - 21:28]
// CHECK-EXPANDED-NEXT:                   `-BraceStmtScope 0x{{.*}}, [21:1 - 21:28]
// CHECK-EXPANDED-NEXT:                     `-PatternEntryDeclScope 0x{{.*}}, [21:5 - 21:28] entry 0 'i'
// CHECK-EXPANDED-NEXT:                       `-PatternEntryInitializerScope 0x{{.*}}, [21:14 - 21:28] entry 0 'i'
