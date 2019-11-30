// REQUIRES: executable_test



// RUN: %empty-directory(%t)
// RUN: cp -r %S/Inputs/common/* %t

// =============================================================================
// Without range dependencies, but logging comparison
// =============================================================================

// RUN: cd %t && %swiftc_driver -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2  >& output1

// =============================================================================
// Compile with range dependencies enabled
// and logging the comparison to comparo.
// =============================================================================


// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes-path=./comparo -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental  >& %t/output2

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output2
// CHECK-HAS-NO-BATCHES-NOT: Batchable: {compile:

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARO-1 %s < %t/comparo
// CHECK-COMPARO-1: *** Incremental build disabled because different arguments passed to compiler, cannot compare ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-TURN-ON %s < %t/output2

// CHECK-TURN-ON-DAG: Incremental compilation has been disabled, because different arguments were passed to the compiler.
// CHECK-TURN-ON-DAG: Added to TaskQueue: {compile: main.o <= main.swift}
// CHECK-TURN-ON-DAG: Added to TaskQueue: {compile: fileA.o <= fileA.swift}
// CHECK-TURN-ON-DAG: Added to TaskQueue: {compile: fileB.o <= fileB.swift}


// RUN: cmp main.swift main.compiledsource
// RUN: cmp fileA.swift fileA.compiledsource
// RUN: cmp fileB.swift fileB.compiledsource

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-MAIN-1 %s <%t/main.swiftranges

// CHECK-MAIN-1: ### Swift source ranges file v0 ###
// CHECK-MAIN-1-NEXT: ---
// CHECK-MAIN-1-NEXT: unparsedRangesByNonPrimary:
// CHECK-MAIN-1-NEXT:   ./fileB.swift:
// CHECK-MAIN-1-NEXT:     - { start: { line: 1, column: 19 }, end: { line: 6, column: 2 } }
// CHECK-MAIN-1-NEXT:     - { start: { line: 7, column: 19 }, end: { line: 10, column: 2 } }
// CHECK-MAIN-1-NEXT: noninlinableFunctionBodies:
// CHECK-MAIN-1-NEXT:  - { start: { line: 5, column: 32 }, end: { line: 5, column: 39 } }
// CHECK-MAIN-1-NEXT: ...

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-FILEA-1 %s <%t/fileA.swiftranges

// CHECK-FILEA-1: ### Swift source ranges file v0 ###
// CHECK-FILEA-1: ---
// CHECK-FILEA-1: unparsedRangesByNonPrimary:
// CHECK-FILEA-1:   ./fileB.swift:
// CHECK-FILEA-1:     - { start: { line: 7, column: 19 }, end: { line: 10, column: 2 } }
// CHECK-FILEA-1: noninlinableFunctionBodies:
// CHECK-FILEA-1:   - { start: { line: 1, column: 17 }, end: { line: 4, column: 2 } }
// CHECK-FILEA-1:   - { start: { line: 5, column: 17 }, end: { line: 7, column: 2 } }
// CHECK-FILEA-1:   - { start: { line: 8, column: 17 }, end: { line: 8, column: 19 } }
// CHECK-FILEA-1: ...


// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-FILEB-1 %s <%t/fileB.swiftranges

// CHECK-FILEB-1: ### Swift source ranges file v0 ###
// CHECK-FILEB-1: ---
// CHECK-FILEB-1: unparsedRangesByNonPrimary: {}
// CHECK-FILEB-1: noninlinableFunctionBodies:
// CHECK-FILEB-1:  - { start: { line: 5, column: 50 }, end: { line: 5, column: 67 } }
// CHECK-FILEB-1: ...

// RUN: %t/main | tee run2 | grep Any > /dev/null && rm %t/main

// =============================================================================
// Steady-state: Now, do it again with range dependencies enabled.
// =============================================================================

// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes-path ./comparo -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output3

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARO-2 %s < %t/comparo
// CHECK-COMPARO-2: *** Incremental build disabled because different arguments passed to compiler, cannot compare ***
// CHECK-COMPARO-2: *** Range benefit: 0 compilations, 0 stages, deps: 0, ranges: 0, total: 3, requested: ranges, used: ranges ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output3


// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-INCREMENTAL-ENABLED %s < %t/output3
// CHECK-INCREMENTAL-ENABLED-NOT: Incremental compilation has been disabled

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-NO-COMPILATION %s < %t/output3

// CHECK-NO-COMPILATION-NOT: Added {{.*}}}} to TaskQueue {{.*}} compile


// RUN: %t/main | tee run3 | grep Any > /dev/null && rm %t/main


// =============================================================================
// Add an attribute to: a structure that no other file uses
// =============================================================================

// RUN: cp %t/comparo %t/saved-comparo


// RUN: cp %t/fileB2.swift %t/fileB.swift
// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output4

// RUN: cmp %t/comparo %t/saved-comparo

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output4

// RUN: %FileCheck -check-prefix=CHECK-FILEB-ONLY %s < %t/output4
// CHECK-FILEB-ONLY-NOT: Queuing{{.*}}<= main.swift
// CHECK-FILEB-ONLY-NOT: Queuing{{.*}}<= fileA.swift

// RUN: %FileCheck -match-full-lines -check-prefix=COMPARE-2-1-3 %s < %t/output4
// COMPARE-2-1-3: *** Range benefit: 1 compilations, 1 stages, deps: 2, ranges: 1, total: 3, requested: ranges, used: ranges ***



// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-FILEB-AND-SELECTING %s < %t/output4

// CHECK-FILEB-AND-SELECTING: Hypothetically: Queuing <Dependencies> (initial): {compile: fileB.o <= fileB.swift}
// CHECK-FILEB-AND-SELECTING: Queuing <Ranges> (this file changed): {compile: fileB.o <= fileB.swift}
// CHECK-FILEB-AND-SELECTING: Using ranges


// RUN: %t/main | tee run4 | grep Any > /dev/null && rm %t/main

// =============================================================================
// Add an attribute to: a structure that one other file uses
// =============================================================================

// RUN: cp %t/fileB3.swift %t/fileB.swift
// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental -driver-dump-compiled-source-diffs >& %t/output5

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output5

// RUN: %FileCheck -check-prefix=CHECK-FILEA-AND-FILEB-ONLY %s < %t/output5
// CHECK-FILEA-AND-FILEB-ONLY-NOT: Queuing{{.*}}<= main.swift

// RUN: %FileCheck -match-full-lines -check-prefix=COMPARE-2-2-3 %s < %t/output5
// COMPARE-2-2-3: *** Range benefit: 0 compilations, 1 stages, deps: 2, ranges: 2, total: 3, requested: ranges, used: ranges ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-DIFFS-NONE-1 %s < %t/output5
// CHECK-DIFFS-NONE-1-DAG: *** no changed ranges in 'main.swift' (w.r.t previously- or about-to-be-compiled) ***
// CHECK-DIFFS-NONE-1-DAG: *** no changed ranges in 'fileA.swift' (w.r.t previously- or about-to-be-compiled) ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-DIFFS-FILEB-1 %s < %t/output5
// CHECK-DIFFS-FILEB-1: *** all changed ranges in 'fileB.swift' (w.r.t previously-compiled) ***
// CHECK-DIFFS-FILEB-1-NEXT: - [4:18--4:19)
// CHECK-DIFFS-FILEB-1-NEXT: *** all changed ranges in 'fileB.swift' (w.r.t to-be-compiled) ***
// CHECK-DIFFS-FILEB-1-NEXT: - [4:18--4:28)
// CHECK-DIFFS-FILEB-1-NEXT: *** nonlocal changed ranges in 'fileB.swift' (w.r.t previously-compiled) ***
// CHECK-DIFFS-FILEB-1-NEXT: - [4:18--4:19)

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-FILEA-FILEB-SELECTING %s < %t/output5

// CHECK-FILEA-FILEB-SELECTING: Hypothetically: Queuing <Dependencies> (initial): {compile: fileB.o <= fileB.swift}
// CHECK-FILEA-FILEB-SELECTING: Queuing <Ranges> (changed: fileB.swift:[4:18--4:19)): {compile: fileA.o <= fileA.swift}
// CHECK-FILEA-FILEB-SELECTING: Queuing <Ranges> (this file changed): {compile: fileB.o <= fileB.swift}
// CHECK-FILEA-FILEB-SELECTING: Using ranges


// RUN: %t/main | tee run5 | grep Any > /dev/null && rm %t/main


// =============================================================================
// What if the user adds a close brace and new type in the middle?
// =============================================================================


// RUN: cp %t/fileB4.swift %t/fileB.swift

// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output6

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output6

// RUN: %FileCheck -check-prefix=CHECK-INITIALLY-ABSENT-MAIN %s < %t/output6
// CHECK-INITIALLY-ABSENT-MAIN-NOT: Queueing{{.*}}<= main.swift

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARE-0-0-3-3-3 %s < %t/output6
// CHECK-COMPARE-0-0-3-3-3:*** Range benefit: 0 compilations, 0 stages, deps: 3, ranges: 3, total: 3, requested: ranges, used: ranges ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-A-B-THEN-MAIN %s < %t/output6

// CHECK-A-B-THEN-MAIN: Hypothetically: Queuing <Dependencies> (initial): {compile: fileB.o <= fileB.swift}
// CHECK-A-B-THEN-MAIN: Queuing <Ranges> (changed: fileB.swift:[5:3--5:67)): {compile: fileA.o <= fileA.swift}
// CHECK-A-B-THEN-MAIN: Queuing <Ranges> (this file changed): {compile: fileB.o <= fileB.swift}
// CHECK-A-B-THEN-MAIN: Using ranges
// CHECK-A-B-THEN-MAIN: After completion of {compile: fileB.o <= fileB.swift}:
// CHECK-A-B-THEN-MAIN-DAG: - Dependencies would now schedule: {compile: main.o <= main.swift}
// CHECK-A-B-THEN-MAIN-DAG: - Dependencies would now schedule: {compile: fileA.o <= fileA.swift}
// CHECK-A-B-THEN-MAIN: - Source ranges will now schedule: {compile: main.o <= main.swift}
// CHECK-A-B-THEN-MAIN: Queuing <Ranges> because of dependencies discovered later: {compile: main.o <= main.swift}

// RUN: %t/main | tee run6 | grep SignedInteger > /dev/null && rm %t/main


// =============================================================================
// What happens when a new file is added?
// =============================================================================

// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift ./fileC.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output7

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output7

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARE-0-0-4-4-4 %s < %t/output7
// CHECK-COMPARE-0-0-4-4-4: *** Range benefit: 0 compilations, 0 stages, deps: 4, ranges (falling back): 4, total: 4, requested: ranges, used: deps ***

// RUN: cmp fileB.swift fileB.compiledsource

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-FILEC-1 %s <%t/fileC.swiftranges
// CHECK-FILEC-1: ### Swift source ranges file v0 ###
// CHECK-FILEC-1-NEXT: ---
// CHECK-FILEC-1-NEXT: unparsedRangesByNonPrimary: {}
// CHECK-FILEC-1-NEXT: noninlinableFunctionBodies:
// CHECK-FILEC-1-NEXT:   - { start: { line: 3, column: 19 }, end: { line: 3, column: 21 } }
// CHECK-FILEC-1-NEXT:   - { start: { line: 5, column: 33 }, end: { line: 5, column: 40 } }
// CHECK-FILEC-1-NEXT: ...

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-ADD-NEW-FILE %s < %t/output7

// CHECK-ADD-NEW-FILE-DAG: <unknown>:0: warning: unable to load swift ranges file "./fileC.swiftranges", No such file or directory
// CHECK-ADD-NEW-FILE-DAG: <unknown>:0: warning: unable to determine when 'fileC.compiledsource' was last modified: No such file or directory
// CHECK-ADD-NEW-FILE-DAG: Hypothetically: Queuing <Dependencies> (initial): {compile: fileC.o <= fileC.swift}
// CHECK-ADD-NEW-FILE-DAG: Using dependencies: At least one input ('fileC.swift') lacks a supplementary output needed for the source range strategy.
// CHECK-ADD-NEW-FILE-DAG: Queuing <Dependencies> because of dependencies discovered later: {compile: fileB.o <= fileB.swift}
// CHECK-ADD-NEW-FILE-DAG: Queuing <Dependencies> because of dependencies discovered later: {compile: fileA.o <= fileA.swift}
// CHECK-ADD-NEW-FILE-DAG: Queuing <Dependencies> because of dependencies discovered later: {compile: main.o <= main.swift}


// RUN: %t/main | tee run7 | grep Int > /dev/null && rm %t/main


// =============================================================================
// How about removing a file?
// =============================================================================

// RUN: cd %t && %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output8

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-FILEC-REMOVED %s < %t/output8

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARE-DISABLED-INPUT-REMOVED %s < %t/output8
// CHECK-COMPARE-DISABLED-INPUT-REMOVED: *** Incremental build disabled because an input was removed, cannot compare ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output8

// CHECK-FILEC-REMOVED: Incremental compilation has been disabled, because the following inputs were used in the previous compilation, but not in the current compilation:
// CHECK-FILEC-REMOVED-NEXT: ./fileC.swift

// RUN: %t/main | tee run8 | grep SignedInteger > /dev/null && rm %t/main


// =============================================================================
// How about an error?
// =============================================================================

// RUN: cp %t/fileB5.swift %t/fileB.swift
// RUN: cd %t && not %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output9

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARE-0-1-3-3-3-DEPS %s < %t/output9
// CHECK-COMPARE-0-1-3-3-3-DEPS: *** Range benefit: 0 compilations, 1 stages, deps: 3, ranges (falling back): 3, total: 3, requested: ranges, used: deps ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output9


// =============================================================================
// And a fix:
// =============================================================================

// RUN: cp %t/fileB4.swift %t/fileB.swift
// RUN: cd %t &&  %swiftc_driver -driver-compare-incremental-schemes -enable-source-range-dependencies -output-file-map %t/output.json -incremental -enable-batch-mode ./main.swift ./fileA.swift ./fileB.swift -module-name main -j2 -driver-show-job-lifecycle -driver-show-incremental >& %t/output10

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-COMPARE-0-0-3-3-3-DEPS %s < %t/output10
// CHECK-COMPARE-0-0-3-3-3-DEPS: *** Range benefit: 0 compilations, 0 stages, deps: 3, ranges (falling back): 3, total: 3, requested: ranges, used: deps ***

// RUN: %FileCheck -match-full-lines -check-prefix=CHECK-HAS-NO-BATCHES  %s < %t/output10



// RUN: %t/main | tee run10 | grep SignedInteger > /dev/null && rm %t/main
