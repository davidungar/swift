// ASTScopes depend on source ranges being correct, so try a bunch here

// All of these tests are line- and
// column-sensitive, so any additions should go at the end.

struct S0 {
  class InnerC0 { }
}

extension S0 {
}

class C0 {
}

enum E0 {
  case C0
  case C1(Int, Int)
}

struct GenericS0<T, U> {
}

func genericFunc0<T, U>(t: T, u: U, i: Int = 10) {
}

class ContainsGenerics0 {
  init<T, U>(t: T, u: U) {
  }

  deinit {
  }
}

typealias GenericAlias0<T> = [T]

#if arch(unknown)
struct UnknownArchStruct { }
#else
struct OtherArchStruct { }
#endif

func functionBodies1(a: Int, b: Int?) {
  let (x1, x2) = (a, b),
      (y1, y2) = (b, a)
  let (z1, z2) = (a, a)
  do {
    let a1 = a
    let a2 = a
    do {
      let b1 = b
      let b2 = b
    }
  }
  do {
    let b1 = b
    let b2 = b
  }
  func f(_ i: Int) -> Int { return i }
  let f2 = f(_:)
  struct S7 { }
  typealias S7Alias = S7

  if let b1 = b, let b2 = b {
    let c1 = b
  } else {
    let c2 = b
  }

  guard let b1 = b, { $0 > 5 }(b1), let b2 = b else {
    let c = 5
    return
  }

  while let b3 = b, let b4 = b {
    let c = 5
  }

  repeat { } while true;

  for (x, y) in [(1, "hello"), (2, "world")] where x % 2 == 0 {

  }

  do {
    try throwing()
  } catch let mine as MyError where mine.value == 17 {
  } catch {
  }

  switch MyEnum.second(1) {
  case .second(let x) where x == 17:
    break;

  case .first:
    break;

  default:
    break;
  }
  for (var i = 0; i != 10; i += 1) { }
}

func throwing() throws { }

struct MyError : Error {
  var value: Int
}

enum MyEnum {
  case first
  case second(Int)
  case third
}

struct StructContainsAbstractStorageDecls {
  subscript (i: Int, j: Int) -> Int {
    set {
    }
    get {
      return i + j
    }
  }

  var computed: Int {
    get {
      return 0
    }
    set {
    }
  }
}

class ClassWithComputedProperties {
  var willSetProperty: Int = 0 {
    willSet { }
  }

  var didSetProperty: Int = 0 {
    didSet { }
  }
}

func funcWithComputedProperties(i: Int) {
  var computed: Int {
    set {
    }
    get {
      return 0
    }
  }, var (stored1, stored2) = (1, 2),
  var alsoComputed: Int {
    return 17
  }

  do { }
}

func closures() {
  { x, y in
    return { $0 + $1 }(x, y)
  }(1, 2) +
  { a, b in a * b }(3, 4)
}

{ closures() }()

func defaultArguments(i: Int = 1,
                      j: Int = { $0 + $1 }(1, 2)) {

  func localWithDefaults(i: Int = 1,
                         j: Int = { $0 + $1 }(1, 2)) {
  }

  let a = i + j
  { $0 }(a)
}

struct PatternInitializers {
  var (a, b) = (1, 2),
      (c, d) = (1.5, 2.5)
}

protocol ProtoWithSubscript {
  subscript(native: Int) -> Int { get set }
}

func localPatternsWithSharedType() {
  let i, j, k: Int
}

class LazyProperties {
  var value: Int = 17

  lazy var prop: Int = self.value
}

typealias KeyPathComputedArgumentLayoutFn =
() -> Int


// RUN: not %target-swift-frontend -typecheck %s -dump-ast  > %t.ast
// RUN: %FileCheck  %s < %t.ast

// CHECK: (struct_decl range=[{{[^:]*}}:6:1 - line:8:1]
// CHECK:   (class_decl range=[{{[^:]*}}:7:3 - line:7:19]
// CHECK:     (destructor_decl implicit range=[{{[^:]*}}:7:9 - line:7:9]
// CHECK:       (brace_stmt implicit range=[{{[^:]*}}:7:9 - line:7:9]
// CHECK:     (constructor_decl implicit range=[{{[^:]*}}:7:9 - line:7:9]
// CHECK:       (brace_stmt implicit range=[{{[^:]*}}:7:9 - line:7:9]
// CHECK:         (return_stmt range=[{{[^:]*}}:7:9 - line:7:9]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:6:8 - line:6:8]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:6:8 - line:6:8]
// CHECK:       (return_stmt range=[{{[^:]*}}:6:8 - line:6:8]
// CHECK: (extension_decl range=[{{[^:]*}}:10:1 - line:11:1]
// CHECK: (class_decl range=[{{[^:]*}}:13:1 - line:14:1]
// CHECK:   (destructor_decl implicit range=[{{[^:]*}}:13:7 - line:13:7]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:13:7 - line:13:7]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:13:7 - line:13:7]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:13:7 - line:13:7]
// CHECK:       (return_stmt range=[{{[^:]*}}:13:7 - line:13:7]
// CHECK: (enum_decl range=[{{[^:]*}}:16:1 - line:19:1]
// CHECK:   (enum_case_decl range=[{{[^:]*}}:17:3 - line:17:8]
// CHECK:     (enum_element_decl range=[{{[^:]*}}:17:8 - line:17:8]
// CHECK:   (enum_element_decl range=[{{[^:]*}}:17:8 - line:17:8]
// CHECK:   (enum_case_decl range=[{{[^:]*}}:18:3 - line:18:19]
// CHECK:     (enum_element_decl range=[{{[^:]*}}:18:8 - line:18:19]
// CHECK:   (enum_element_decl range=[{{[^:]*}}:18:8 - line:18:19]
// CHECK: (struct_decl range=[{{[^:]*}}:21:1 - line:22:1]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:21:8 - line:21:8]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:21:8 - line:21:8]
// CHECK:       (return_stmt range=[{{[^:]*}}:21:8 - line:21:8]
// CHECK: (func_decl range=[{{[^:]*}}:24:1 - line:25:1]
// CHECK:       (integer_literal_expr type='Int' location={{[^:]*}}:24:46 range=[{{[^:]*}}:24:46 - line:24:46]
// CHECK:   (brace_stmt range=[{{[^:]*}}:24:50 - line:25:1]
// CHECK: (class_decl range=[{{[^:]*}}:27:1 - line:33:1]
// CHECK:   (constructor_decl range=[{{[^:]*}}:28:3 - line:29:3]
// CHECK:       (parameter "u" apiName=u interface type='U') range=[{{[^:]*}}:28:13 - line:28:24]
// CHECK:     (brace_stmt range=[{{[^:]*}}:28:26 - line:29:3]
// CHECK:       (return_stmt implicit range=[{{[^:]*}}:29:3 - line:29:3]
// CHECK:   (destructor_decl range=[{{[^:]*}}:31:3 - line:32:3]
// CHECK:     (brace_stmt range=[{{[^:]*}}:31:10 - line:32:3]
// CHECK: (typealias range=[{{[^:]*}}:35:1 - line:35:32]
// CHECK: (if_config_decl range=[{{[^:]*}}:37:1 - line:41:1]
// CHECK:       (struct_decl range=[{{[^:]*}}:38:1 - line:38:28]
// CHECK:       (struct_decl range=[{{[^:]*}}:40:1 - line:40:26]
// CHECK:         (constructor_decl implicit range=[{{[^:]*}}:40:8 - line:40:8]
// CHECK:           (brace_stmt implicit range=[{{[^:]*}}:40:8 - line:40:8]
// CHECK:             (return_stmt range=[{{[^:]*}}:40:8 - line:40:8]
// CHECK: (struct_decl range=[{{[^:]*}}:40:1 - line:40:26]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:40:8 - line:40:8]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:40:8 - line:40:8]
// CHECK:       (return_stmt range=[{{[^:]*}}:40:8 - line:40:8]
// CHECK: (func_decl range=[{{[^:]*}}:43:1 - line:102:1]
// CHECK:     (parameter "b" apiName=b interface type='Int?') range=[{{[^:]*}}:43:21 - line:43:37]
// CHECK:   (brace_stmt range=[{{[^:]*}}:43:39 - line:102:1]
// CHECK:     (pattern_binding_decl range=[{{[^:]*}}:44:3 - line:45:23]
// CHECK:       (tuple_expr type='(Int, Int?)' location={{[^:]*}}:44:18 range=[{{[^:]*}}:44:18 - line:44:23]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:44:19 range=[{{[^:]*}}:44:19 - line:44:19]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:44:22 range=[{{[^:]*}}:44:22 - line:44:22]
// CHECK:       (tuple_expr type='(Int, Int?)' location={{[^:]*}}:44:18 range=[{{[^:]*}}:44:18 - line:44:23]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:44:19 range=[{{[^:]*}}:44:19 - line:44:19]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:44:22 range=[{{[^:]*}}:44:22 - line:44:22]
// CHECK:       (tuple_expr type='(Int?, Int)' location={{[^:]*}}:45:18 range=[{{[^:]*}}:45:18 - line:45:23]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:45:19 range=[{{[^:]*}}:45:19 - line:45:19]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:45:22 range=[{{[^:]*}}:45:22 - line:45:22]
// CHECK:       (tuple_expr type='(Int?, Int)' location={{[^:]*}}:45:18 range=[{{[^:]*}}:45:18 - line:45:23]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:45:19 range=[{{[^:]*}}:45:19 - line:45:19]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:45:22 range=[{{[^:]*}}:45:22 - line:45:22]
// CHECK:     (var_decl range=[{{[^:]*}}:44:8 - line:44:8]
// CHECK:     (var_decl range=[{{[^:]*}}:44:12 - line:44:12]
// CHECK:     (var_decl range=[{{[^:]*}}:45:8 - line:45:8]
// CHECK:     (var_decl range=[{{[^:]*}}:45:12 - line:45:12]
// CHECK:     (pattern_binding_decl range=[{{[^:]*}}:46:3 - line:46:23]
// CHECK:       (tuple_expr type='(Int, Int)' location={{[^:]*}}:46:18 range=[{{[^:]*}}:46:18 - line:46:23]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:46:19 range=[{{[^:]*}}:46:19 - line:46:19]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:46:22 range=[{{[^:]*}}:46:22 - line:46:22]
// CHECK:       (tuple_expr type='(Int, Int)' location={{[^:]*}}:46:18 range=[{{[^:]*}}:46:18 - line:46:23]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:46:19 range=[{{[^:]*}}:46:19 - line:46:19]
// CHECK:         (declref_expr type='Int' location={{[^:]*}}:46:22 range=[{{[^:]*}}:46:22 - line:46:22]
// CHECK:     (var_decl range=[{{[^:]*}}:46:8 - line:46:8]
// CHECK:     (var_decl range=[{{[^:]*}}:46:12 - line:46:12]
// CHECK:     (do_stmt range=[{{[^:]*}}:47:3 - line:54:3]
// CHECK:       (brace_stmt range=[{{[^:]*}}:47:6 - line:54:3]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:48:5 - line:48:14]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).a@{{[^:]*}}:43:22 function_ref=unapplied)
// CHECK:           (declref_expr type='Int' location={{[^:]*}}:48:14 range=[{{[^:]*}}:48:14 - line:48:14]
// CHECK:         (var_decl range=[{{[^:]*}}:48:9 - line:48:9]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:49:5 - line:49:14]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).a@{{[^:]*}}:43:22 function_ref=unapplied)
// CHECK:           (declref_expr type='Int' location={{[^:]*}}:49:14 range=[{{[^:]*}}:49:14 - line:49:14]
// CHECK:         (var_decl range=[{{[^:]*}}:49:9 - line:49:9]
// CHECK:         (do_stmt range=[{{[^:]*}}:50:5 - line:53:5]
// CHECK:           (brace_stmt range=[{{[^:]*}}:50:8 - line:53:5]
// CHECK:             (pattern_binding_decl range=[{{[^:]*}}:51:7 - line:51:16]
// CHECK:               (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).b@{{[^:]*}}:43:30 function_ref=unapplied)
// CHECK:               (declref_expr type='Int?' location={{[^:]*}}:51:16 range=[{{[^:]*}}:51:16 - line:51:16]
// CHECK:             (var_decl range=[{{[^:]*}}:51:11 - line:51:11]
// CHECK:             (pattern_binding_decl range=[{{[^:]*}}:52:7 - line:52:16]
// CHECK:               (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).b@{{[^:]*}}:43:30 function_ref=unapplied)
// CHECK:               (declref_expr type='Int?' location={{[^:]*}}:52:16 range=[{{[^:]*}}:52:16 - line:52:16]
// CHECK:             (var_decl range=[{{[^:]*}}:52:11 - line:52:11]
// CHECK:     (do_stmt range=[{{[^:]*}}:55:3 - line:58:3]
// CHECK:       (brace_stmt range=[{{[^:]*}}:55:6 - line:58:3]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:56:5 - line:56:14]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).b@{{[^:]*}}:43:30 function_ref=unapplied)
// CHECK:           (declref_expr type='Int?' location={{[^:]*}}:56:14 range=[{{[^:]*}}:56:14 - line:56:14]
// CHECK:         (var_decl range=[{{[^:]*}}:56:9 - line:56:9]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:57:5 - line:57:14]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).b@{{[^:]*}}:43:30 function_ref=unapplied)
// CHECK:           (declref_expr type='Int?' location={{[^:]*}}:57:14 range=[{{[^:]*}}:57:14 - line:57:14]
// CHECK:         (var_decl range=[{{[^:]*}}:57:9 - line:57:9]
// CHECK:     (func_decl range=[{{[^:]*}}:59:3 - line:59:38]
// CHECK:         (parameter "i" interface type='Int') range=[{{[^:]*}}:59:9 - line:59:18]
// CHECK:       (brace_stmt range=[{{[^:]*}}:59:27 - line:59:38]
// CHECK:         (return_stmt range=[{{[^:]*}}:59:29 - line:59:36]
// CHECK:           (declref_expr type='Int' location={{[^:]*}}:59:36 range=[{{[^:]*}}:59:36 - line:59:36]
// CHECK:     (pattern_binding_decl range=[{{[^:]*}}:60:3 - line:60:16]
// CHECK:       (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).f@{{[^:]*}}:59:8 function_ref=compound)
// CHECK:       (declref_expr type='(Int) -> Int' location={{[^:]*}}:60:12 range=[{{[^:]*}}:60:12 - line:60:16]
// CHECK:     (var_decl range=[{{[^:]*}}:60:7 - line:60:7]
// CHECK:     (struct_decl range=[{{[^:]*}}:61:3 - line:61:15]
// CHECK:       (constructor_decl implicit range=[{{[^:]*}}:61:10 - line:61:10]
// CHECK:         (brace_stmt implicit range=[{{[^:]*}}:61:10 - line:61:10]
// CHECK:           (return_stmt range=[{{[^:]*}}:61:10 - line:61:10]
// CHECK:     (typealias range=[{{[^:]*}}:62:3 - line:62:23]
// CHECK:     (if_stmt range=[{{[^:]*}}:64:3 - line:68:3]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:64:15 range=[{{[^:]*}}:64:15 - line:64:15]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:64:27 range=[{{[^:]*}}:64:27 - line:64:27]
// CHECK:       (brace_stmt range=[{{[^:]*}}:64:29 - line:66:3]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:65:5 - line:65:14]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).b@{{[^:]*}}:43:30 function_ref=unapplied)
// CHECK:           (declref_expr type='Int?' location={{[^:]*}}:65:14 range=[{{[^:]*}}:65:14 - line:65:14]
// CHECK:         (var_decl range=[{{[^:]*}}:65:9 - line:65:9]
// CHECK:       (brace_stmt range=[{{[^:]*}}:66:10 - line:68:3]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:67:5 - line:67:14]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).functionBodies1(a:b:).b@{{[^:]*}}:43:30 function_ref=unapplied)
// CHECK:           (declref_expr type='Int?' location={{[^:]*}}:67:14 range=[{{[^:]*}}:67:14 - line:67:14]
// CHECK:         (var_decl range=[{{[^:]*}}:67:9 - line:67:9]
// CHECK:     (guard_stmt range=[{{[^:]*}}:70:3 - line:73:3]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:70:18 range=[{{[^:]*}}:70:18 - line:70:18]
// CHECK:         (closure_expr type='(Int) -> Bool' location={{[^:]*}}:70:21 range=[{{[^:]*}}:70:21 - line:70:30]
// CHECK:             (parameter "$0" type='Int' interface type='Int') range=[{{[^:]*}}:70:21 - line:70:21]
// CHECK:           (binary_expr type='Bool' location={{[^:]*}}:70:26 range=[{{[^:]*}}:70:23 - line:70:28]
// CHECK:             (dot_syntax_call_expr implicit type='(Int, Int) -> Bool' location={{[^:]*}}:70:26 range=[{{[^:]*}}:70:26 - line:70:26]
// CHECK:               (declref_expr type='(Int.Type) -> (Int, Int) -> Bool' location={{[^:]*}}:70:26 range=[{{[^:]*}}:70:26 - line:70:26]
// CHECK:               (type_expr implicit type='Int.Type' location={{[^:]*}}:70:26 range=[{{[^:]*}}:70:26 - line:70:26]
// CHECK:             (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:70:23 range=[{{[^:]*}}:70:23 - line:70:28]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:70:23 range=[{{[^:]*}}:70:23 - line:70:23]
// CHECK:               (integer_literal_expr type='Int' location={{[^:]*}}:70:28 range=[{{[^:]*}}:70:28 - line:70:28]
// CHECK:         (paren_expr type='(Int)' location={{[^:]*}}:70:32 range=[{{[^:]*}}:70:31 - line:70:34]
// CHECK:           (declref_expr type='Int' location={{[^:]*}}:70:32 range=[{{[^:]*}}:70:32 - line:70:32]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:70:46 range=[{{[^:]*}}:70:46 - line:70:46]
// CHECK:       (brace_stmt range=[{{[^:]*}}:70:53 - line:73:3]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:71:5 - line:71:13]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:71:13 range=[{{[^:]*}}:71:13 - line:71:13]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:71:13 range=[{{[^:]*}}:71:13 - line:71:13]
// CHECK:         (var_decl range=[{{[^:]*}}:71:9 - line:71:9]
// CHECK:         (return_stmt range=[{{[^:]*}}:72:5 - line:72:5]
// CHECK:     (while_stmt range=[{{[^:]*}}:75:3 - line:77:3]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:75:18 range=[{{[^:]*}}:75:18 - line:75:18]
// CHECK:         (declref_expr type='Int?' location={{[^:]*}}:75:30 range=[{{[^:]*}}:75:30 - line:75:30]
// CHECK:       (brace_stmt range=[{{[^:]*}}:75:32 - line:77:3]
// CHECK:         (pattern_binding_decl range=[{{[^:]*}}:76:5 - line:76:13]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:76:13 range=[{{[^:]*}}:76:13 - line:76:13]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:76:13 range=[{{[^:]*}}:76:13 - line:76:13]
// CHECK:         (var_decl range=[{{[^:]*}}:76:9 - line:76:9]
// CHECK:     (repeat_while_stmt range=[{{[^:]*}}:79:3 - line:79:20]
// CHECK:       (brace_stmt range=[{{[^:]*}}:79:10 - line:79:12]
// CHECK:       (boolean_literal_expr type='Bool' location={{[^:]*}}:79:20 range=[{{[^:]*}}:79:20 - line:79:20]
// CHECK:     (for_each_stmt range=[{{[^:]*}}:81:3 - line:83:3]
// CHECK:         (binary_expr type='Bool' location={{[^:]*}}:81:58 range=[{{[^:]*}}:81:52 - line:81:61]
// CHECK:           (dot_syntax_call_expr implicit type='(Int, Int) -> Bool' location={{[^:]*}}:81:58 range=[{{[^:]*}}:81:58 - line:81:58]
// CHECK:             (declref_expr type='(Int.Type) -> (Int, Int) -> Bool' location={{[^:]*}}:81:58 range=[{{[^:]*}}:81:58 - line:81:58]
// CHECK:             (type_expr implicit type='Int.Type' location={{[^:]*}}:81:58 range=[{{[^:]*}}:81:58 - line:81:58]
// CHECK:           (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:81:52 range=[{{[^:]*}}:81:52 - line:81:61]
// CHECK:             (binary_expr type='Int' location={{[^:]*}}:81:54 range=[{{[^:]*}}:81:52 - line:81:56]
// CHECK:               (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:81:54 range=[{{[^:]*}}:81:54 - line:81:54]
// CHECK:                 (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:81:54 range=[{{[^:]*}}:81:54 - line:81:54]
// CHECK:                 (type_expr implicit type='Int.Type' location={{[^:]*}}:81:54 range=[{{[^:]*}}:81:54 - line:81:54]
// CHECK:               (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:81:52 range=[{{[^:]*}}:81:52 - line:81:56]
// CHECK:                 (declref_expr type='Int' location={{[^:]*}}:81:52 range=[{{[^:]*}}:81:52 - line:81:52]
// CHECK:                 (integer_literal_expr type='Int' location={{[^:]*}}:81:56 range=[{{[^:]*}}:81:56 - line:81:56]
// CHECK:             (integer_literal_expr type='Int' location={{[^:]*}}:81:61 range=[{{[^:]*}}:81:61 - line:81:61]
// CHECK:       (array_expr type='[(Int, String)]
// CHECK:         (tuple_expr type='(Int, String)' location={{[^:]*}}:81:18 range=[{{[^:]*}}:81:18 - line:81:29]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:81:19 range=[{{[^:]*}}:81:19 - line:81:19]
// CHECK:           (string_literal_expr type='String' location={{[^:]*}}:81:22 range=[{{[^:]*}}:81:22 - line:81:22]
// CHECK:         (tuple_expr type='(Int, String)' location={{[^:]*}}:81:32 range=[{{[^:]*}}:81:32 - line:81:43]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:81:33 range=[{{[^:]*}}:81:33 - line:81:33]
// CHECK:           (string_literal_expr type='String' location={{[^:]*}}:81:36 range=[{{[^:]*}}:81:36 - line:81:36]
// CHECK:       (var_decl implicit range=[{{[^:]*}}:81:14 - line:81:14]
// CHECK:       (declref_expr implicit type='@lvalue IndexingIterator<Array<(Int, String)>>' location={{[^:]*}}:81:14 range=[{{[^:]*}}:81:14 - line:81:14]
// CHECK:       (brace_stmt range=[{{[^:]*}}:81:63 - line:83:3]
// CHECK:     (do_catch_stmt range=[{{[^:]*}}:85:3 - line:89:3]
// CHECK:       (brace_stmt range=[{{[^:]*}}:85:6 - line:87:3]
// CHECK:         (try_expr type='()' location={{[^:]*}}:86:9 range=[{{[^:]*}}:86:5 - line:86:18]
// CHECK:           (call_expr type='()' location={{[^:]*}}:86:9 range=[{{[^:]*}}:86:9 - line:86:18]
// CHECK:             (declref_expr type='() throws -> ()' location={{[^:]*}}:86:9 range=[{{[^:]*}}:86:9 - line:86:9]
// CHECK:             (tuple_expr type='()' location={{[^:]*}}:86:17 range=[{{[^:]*}}:86:17 - line:86:18]
// CHECK:       (catch range=[{{[^:]*}}:87:5 - line:88:3]
// CHECK:         (binary_expr type='Bool' location={{[^:]*}}:87:48 range=[{{[^:]*}}:87:37 - line:87:51]
// CHECK:           (dot_syntax_call_expr implicit type='(Int, Int) -> Bool' location={{[^:]*}}:87:48 range=[{{[^:]*}}:87:48 - line:87:48]
// CHECK:             (declref_expr type='(Int.Type) -> (Int, Int) -> Bool' location={{[^:]*}}:87:48 range=[{{[^:]*}}:87:48 - line:87:48]
// CHECK:             (type_expr implicit type='Int.Type' location={{[^:]*}}:87:48 range=[{{[^:]*}}:87:48 - line:87:48]
// CHECK:           (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:87:37 range=[{{[^:]*}}:87:37 - line:87:51]
// CHECK:             (member_ref_expr type='Int' location={{[^:]*}}:87:42 range=[{{[^:]*}}:87:37 - line:87:42]
// CHECK:               (declref_expr type='MyError' location={{[^:]*}}:87:37 range=[{{[^:]*}}:87:37 - line:87:37]
// CHECK:             (integer_literal_expr type='Int' location={{[^:]*}}:87:51 range=[{{[^:]*}}:87:51 - line:87:51]
// CHECK:         (brace_stmt range=[{{[^:]*}}:87:54 - line:88:3]
// CHECK:         (brace_stmt range=[{{[^:]*}}:88:11 - line:89:3]
// CHECK:     (switch_stmt range=[{{[^:]*}}:91:3 - line:100:3]
// CHECK:       (call_expr type='MyEnum' location={{[^:]*}}:91:17 range=[{{[^:]*}}:91:10 - line:91:25]
// CHECK:         (dot_syntax_call_expr type='(Int) -> MyEnum' location={{[^:]*}}:91:17 range=[{{[^:]*}}:91:10 - line:91:17]
// CHECK:           (declref_expr type='(MyEnum.Type) -> (Int) -> MyEnum' location={{[^:]*}}:91:17 range=[{{[^:]*}}:91:17 - line:91:17]
// CHECK:           (type_expr type='MyEnum.Type' location={{[^:]*}}:91:10 range=[{{[^:]*}}:91:10 - line:91:10]
// CHECK:         (paren_expr type='(Int)' location={{[^:]*}}:91:24 range=[{{[^:]*}}:91:23 - line:91:25]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:91:24 range=[{{[^:]*}}:91:24 - line:91:24]
// CHECK:       (case_stmt range=[{{[^:]*}}:92:3 - line:93:10]
// CHECK:           (var_decl implicit range=[{{[^:]*}}:92:20 - line:92:20]
// CHECK:           (binary_expr type='Bool' location={{[^:]*}}:92:31 range=[{{[^:]*}}:92:29 - line:92:34]
// CHECK:             (dot_syntax_call_expr implicit type='(Int, Int) -> Bool' location={{[^:]*}}:92:31 range=[{{[^:]*}}:92:31 - line:92:31]
// CHECK:               (declref_expr type='(Int.Type) -> (Int, Int) -> Bool' location={{[^:]*}}:92:31 range=[{{[^:]*}}:92:31 - line:92:31]
// CHECK:               (type_expr implicit type='Int.Type' location={{[^:]*}}:92:31 range=[{{[^:]*}}:92:31 - line:92:31]
// CHECK:             (tuple_expr implicit type='((Int), Int)' location={{[^:]*}}:92:29 range=[{{[^:]*}}:92:29 - line:92:34]
// CHECK:               (declref_expr type='(Int)' location={{[^:]*}}:92:29 range=[{{[^:]*}}:92:29 - line:92:29]
// CHECK:               (integer_literal_expr type='Int' location={{[^:]*}}:92:34 range=[{{[^:]*}}:92:34 - line:92:34]
// CHECK:         (brace_stmt implicit range=[{{[^:]*}}:93:5 - line:93:10]
// CHECK:           (break_stmt range=[{{[^:]*}}:93:5 - line:93:5]
// CHECK:       (case_stmt range=[{{[^:]*}}:95:3 - line:96:10]
// CHECK:         (brace_stmt implicit range=[{{[^:]*}}:96:5 - line:96:10]
// CHECK:           (break_stmt range=[{{[^:]*}}:96:5 - line:96:5]
// CHECK:       (case_stmt range=[{{[^:]*}}:98:3 - line:99:10]
// CHECK:         (brace_stmt implicit range=[{{[^:]*}}:99:5 - line:99:10]
// CHECK:           (break_stmt range=[{{[^:]*}}:99:5 - line:99:5]
// CHECK:     (for_each_stmt range=[{{[^:]*}}:101:3 - line:101:38]
// CHECK:       (brace_stmt range=[{{[^:]*}}:101:36 - line:101:38]
// CHECK: (func_decl range=[{{[^:]*}}:104:1 - line:104:26]
// CHECK:   (parameter_list range=[{{[^:]*}}:104:14 - line:104:15]
// CHECK:   (brace_stmt range=[{{[^:]*}}:104:24 - line:104:26]
// CHECK: (struct_decl range=[{{[^:]*}}:106:1 - line:108:1]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:107:3 - line:107:14]
// CHECK:   (var_decl range=[{{[^:]*}}:107:7 - line:107:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:107:7 - line:107:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:107:7 - line:107:7]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:107:7 - line:107:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:107:7 - line:107:7]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:106:8 - line:106:8]
// CHECK:       (parameter "value" apiName=value interface type='Int') range=[{{[^:]*}}:106:8 - line:106:8]
// CHECK: (enum_decl range=[{{[^:]*}}:110:1 - line:114:1]
// CHECK:   (enum_case_decl range=[{{[^:]*}}:111:3 - line:111:8]
// CHECK:     (enum_element_decl range=[{{[^:]*}}:111:8 - line:111:8]
// CHECK:   (enum_element_decl range=[{{[^:]*}}:111:8 - line:111:8]
// CHECK:   (enum_case_decl range=[{{[^:]*}}:112:3 - line:112:18]
// CHECK:     (enum_element_decl range=[{{[^:]*}}:112:8 - line:112:18]
// CHECK:   (enum_element_decl range=[{{[^:]*}}:112:8 - line:112:18]
// CHECK:   (enum_case_decl range=[{{[^:]*}}:113:3 - line:113:8]
// CHECK:     (enum_element_decl range=[{{[^:]*}}:113:8 - line:113:8]
// CHECK:   (enum_element_decl range=[{{[^:]*}}:113:8 - line:113:8]
// CHECK: (struct_decl range=[{{[^:]*}}:116:1 - line:132:1]
// CHECK:   (subscript_decl range=[{{[^:]*}}:117:3 - line:123:3]
// CHECK:     (accessor_decl range=[{{[^:]*}}:118:5 - line:119:5]
// CHECK:         (parameter "j" interface type='Int') range=[{{[^:]*}}:118:5 - line:118:5]
// CHECK:       (brace_stmt range=[{{[^:]*}}:118:9 - line:119:5]
// CHECK:     (accessor_decl range=[{{[^:]*}}:120:5 - line:122:5]
// CHECK:         (parameter "j" interface type='Int') range=[{{[^:]*}}:117:13 - line:117:28]
// CHECK:       (brace_stmt range=[{{[^:]*}}:120:9 - line:122:5]
// CHECK:         (return_stmt range=[{{[^:]*}}:121:7 - line:121:18]
// CHECK:           (binary_expr type='Int' location={{[^:]*}}:121:16 range=[{{[^:]*}}:121:14 - line:121:18]
// CHECK:             (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:121:16 range=[{{[^:]*}}:121:16 - line:121:16]
// CHECK:               (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:121:16 range=[{{[^:]*}}:121:16 - line:121:16]
// CHECK:               (type_expr implicit type='Int.Type' location={{[^:]*}}:121:16 range=[{{[^:]*}}:121:16 - line:121:16]
// CHECK:             (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:121:14 range=[{{[^:]*}}:121:14 - line:121:18]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:121:14 range=[{{[^:]*}}:121:14 - line:121:14]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:121:18 range=[{{[^:]*}}:121:18 - line:121:18]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:117:3 - line:117:3]
// CHECK:         (parameter "j" interface type='Int') range=[{{[^:]*}}:117:14 - line:117:25]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:125:3 - line:131:3]
// CHECK:   (var_decl range=[{{[^:]*}}:125:7 - line:125:7]
// CHECK:     (accessor_decl range=[{{[^:]*}}:126:5 - line:128:5]
// CHECK:       (brace_stmt range=[{{[^:]*}}:126:9 - line:128:5]
// CHECK:         (return_stmt range=[{{[^:]*}}:127:7 - line:127:14]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:127:14 range=[{{[^:]*}}:127:14 - line:127:14]
// CHECK:     (accessor_decl range=[{{[^:]*}}:129:5 - line:130:5]
// CHECK:         (parameter "newValue" interface type='Int') range=[{{[^:]*}}:129:5 - line:129:5]
// CHECK:       (brace_stmt range=[{{[^:]*}}:129:9 - line:130:5]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:125:7 - line:125:7]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:116:8 - line:116:8]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:116:8 - line:116:8]
// CHECK:       (return_stmt range=[{{[^:]*}}:116:8 - line:116:8]
// CHECK: (class_decl range=[{{[^:]*}}:134:1 - line:142:1]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:135:3 - line:137:3]
// CHECK:     (integer_literal_expr type='Int' location={{[^:]*}}:135:30 range=[{{[^:]*}}:135:30 - line:135:30]
// CHECK:     (integer_literal_expr type='Int' location={{[^:]*}}:135:30 range=[{{[^:]*}}:135:30 - line:135:30]
// CHECK:   (var_decl range=[{{[^:]*}}:135:7 - line:135:7]
// CHECK:     (accessor_decl range=[{{[^:]*}}:136:5 - line:136:15]
// CHECK:         (parameter "newValue" interface type='Int') range=[{{[^:]*}}:136:5 - line:136:5]
// CHECK:       (brace_stmt range=[{{[^:]*}}:136:13 - line:136:15]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:135:7 - line:135:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:135:7 - line:135:7]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:135:7 - line:135:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:135:7 - line:135:7]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:139:3 - line:141:3]
// CHECK:     (integer_literal_expr type='Int' location={{[^:]*}}:139:29 range=[{{[^:]*}}:139:29 - line:139:29]
// CHECK:     (integer_literal_expr type='Int' location={{[^:]*}}:139:29 range=[{{[^:]*}}:139:29 - line:139:29]
// CHECK:   (var_decl range=[{{[^:]*}}:139:7 - line:139:7]
// CHECK:     (accessor_decl range=[{{[^:]*}}:140:5 - line:140:14]
// CHECK:         (parameter "oldValue" interface type='Int') range=[{{[^:]*}}:140:5 - line:140:5]
// CHECK:       (brace_stmt range=[{{[^:]*}}:140:12 - line:140:14]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:139:7 - line:139:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:139:7 - line:139:7]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:139:7 - line:139:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:139:7 - line:139:7]
// CHECK:   (destructor_decl implicit range=[{{[^:]*}}:134:7 - line:134:7]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:134:7 - line:134:7]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:134:7 - line:134:7]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:134:7 - line:134:7]
// CHECK:       (return_stmt range=[{{[^:]*}}:134:7 - line:134:7]
// CHECK: (func_decl range=[{{[^:]*}}:144:1 - line:157:1]
// CHECK:     (parameter "i" apiName=i interface type='Int') range=[{{[^:]*}}:144:32 - line:144:39]
// CHECK:   (brace_stmt range=[{{[^:]*}}:144:41 - line:157:1]
// CHECK:     (pattern_binding_decl range=[{{[^:]*}}:145:3 - line:154:3]
// CHECK:       (tuple_expr type='(Int, Int)' location={{[^:]*}}:151:31 range=[{{[^:]*}}:151:31 - line:151:36]
// CHECK:         (integer_literal_expr type='Int' location={{[^:]*}}:151:32 range=[{{[^:]*}}:151:32 - line:151:32]
// CHECK:         (integer_literal_expr type='Int' location={{[^:]*}}:151:35 range=[{{[^:]*}}:151:35 - line:151:35]
// CHECK:       (tuple_expr type='(Int, Int)' location={{[^:]*}}:151:31 range=[{{[^:]*}}:151:31 - line:151:36]
// CHECK:         (integer_literal_expr type='Int' location={{[^:]*}}:151:32 range=[{{[^:]*}}:151:32 - line:151:32]
// CHECK:         (integer_literal_expr type='Int' location={{[^:]*}}:151:35 range=[{{[^:]*}}:151:35 - line:151:35]
// CHECK:     (var_decl range=[{{[^:]*}}:145:7 - line:145:7]
// CHECK:       (accessor_decl range=[{{[^:]*}}:146:5 - line:147:5]
// CHECK:           (parameter "newValue" interface type='Int') range=[{{[^:]*}}:146:5 - line:146:5]
// CHECK:         (brace_stmt range=[{{[^:]*}}:146:9 - line:147:5]
// CHECK:       (accessor_decl range=[{{[^:]*}}:148:5 - line:150:5]
// CHECK:         (brace_stmt range=[{{[^:]*}}:148:9 - line:150:5]
// CHECK:           (return_stmt range=[{{[^:]*}}:149:7 - line:149:14]
// CHECK:             (integer_literal_expr type='Int' location={{[^:]*}}:149:14 range=[{{[^:]*}}:149:14 - line:149:14]
// CHECK:     (var_decl range=[{{[^:]*}}:151:11 - line:151:11]
// CHECK:     (var_decl range=[{{[^:]*}}:151:20 - line:151:20]
// CHECK:     (var_decl range=[{{[^:]*}}:152:7 - line:152:7]
// CHECK:       (accessor_decl range=[{{[^:]*}}:152:25 - line:154:3]
// CHECK:         (brace_stmt range=[{{[^:]*}}:152:25 - line:154:3]
// CHECK:           (return_stmt range=[{{[^:]*}}:153:5 - line:153:12]
// CHECK:             (integer_literal_expr type='Int' location={{[^:]*}}:153:12 range=[{{[^:]*}}:153:12 - line:153:12]
// CHECK:     (do_stmt range=[{{[^:]*}}:156:3 - line:156:8]
// CHECK:       (brace_stmt range=[{{[^:]*}}:156:6 - line:156:8]
// CHECK: (func_decl range=[{{[^:]*}}:159:1 - line:164:1]
// CHECK:   (parameter_list range=[{{[^:]*}}:159:14 - line:159:15]
// CHECK:   (binary_expr type='Int' location={{[^:]*}}:162:11 range=[{{[^:]*}}:160:3 - line:163:25]
// CHECK:     (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:162:11 range=[{{[^:]*}}:162:11 - line:162:11]
// CHECK:       (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:162:11 range=[{{[^:]*}}:162:11 - line:162:11]
// CHECK:       (type_expr implicit type='Int.Type' location={{[^:]*}}:162:11 range=[{{[^:]*}}:162:11 - line:162:11]
// CHECK:     (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:160:3 range=[{{[^:]*}}:160:3 - line:163:25]
// CHECK:       (call_expr type='Int' location={{[^:]*}}:160:3 range=[{{[^:]*}}:160:3 - line:162:9]
// CHECK:         (closure_expr type='(Int, Int) -> Int' location={{[^:]*}}:160:3 range=[{{[^:]*}}:160:3 - line:162:3]
// CHECK:             (parameter "y" type='Int' interface type='Int') range=[{{[^:]*}}:160:5 - line:160:8]
// CHECK:           (call_expr type='Int' location={{[^:]*}}:161:12 range=[{{[^:]*}}:161:12 - line:161:28]
// CHECK:             (closure_expr type='(Int, Int) -> Int' location={{[^:]*}}:161:12 range=[{{[^:]*}}:161:12 - line:161:22]
// CHECK:                 (parameter "$1" type='Int' interface type='Int') range=[{{[^:]*}}:161:12 - line:161:12]
// CHECK:               (binary_expr type='Int' location={{[^:]*}}:161:17 range=[{{[^:]*}}:161:14 - line:161:19]
// CHECK:                 (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:161:17 range=[{{[^:]*}}:161:17 - line:161:17]
// CHECK:                   (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:161:17 range=[{{[^:]*}}:161:17 - line:161:17]
// CHECK:                   (type_expr implicit type='Int.Type' location={{[^:]*}}:161:17 range=[{{[^:]*}}:161:17 - line:161:17]
// CHECK:                 (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:161:14 range=[{{[^:]*}}:161:14 - line:161:19]
// CHECK:                   (declref_expr type='Int' location={{[^:]*}}:161:14 range=[{{[^:]*}}:161:14 - line:161:14]
// CHECK:                   (declref_expr type='Int' location={{[^:]*}}:161:19 range=[{{[^:]*}}:161:19 - line:161:19]
// CHECK:             (tuple_expr type='(Int, Int)' location={{[^:]*}}:161:23 range=[{{[^:]*}}:161:23 - line:161:28]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:161:24 range=[{{[^:]*}}:161:24 - line:161:24]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:161:27 range=[{{[^:]*}}:161:27 - line:161:27]
// CHECK:         (tuple_expr type='(Int, Int)' location={{[^:]*}}:162:4 range=[{{[^:]*}}:162:4 - line:162:9]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:162:5 range=[{{[^:]*}}:162:5 - line:162:5]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:162:8 range=[{{[^:]*}}:162:8 - line:162:8]
// CHECK:       (call_expr type='Int' location={{[^:]*}}:163:3 range=[{{[^:]*}}:163:3 - line:163:25]
// CHECK:         (closure_expr type='(Int, Int) -> Int' location={{[^:]*}}:163:3 range=[{{[^:]*}}:163:3 - line:163:19]
// CHECK:             (parameter "b" type='Int' interface type='Int') range=[{{[^:]*}}:163:5 - line:163:8]
// CHECK:           (binary_expr type='Int' location={{[^:]*}}:163:15 range=[{{[^:]*}}:163:13 - line:163:17]
// CHECK:             (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:163:15 range=[{{[^:]*}}:163:15 - line:163:15]
// CHECK:               (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:163:15 range=[{{[^:]*}}:163:15 - line:163:15]
// CHECK:               (type_expr implicit type='Int.Type' location={{[^:]*}}:163:15 range=[{{[^:]*}}:163:15 - line:163:15]
// CHECK:             (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:163:13 range=[{{[^:]*}}:163:13 - line:163:17]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:163:13 range=[{{[^:]*}}:163:13 - line:163:13]
// CHECK:               (declref_expr type='Int' location={{[^:]*}}:163:17 range=[{{[^:]*}}:163:17 - line:163:17]
// CHECK:         (tuple_expr type='(Int, Int)' location={{[^:]*}}:163:20 range=[{{[^:]*}}:163:20 - line:163:25]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:163:21 range=[{{[^:]*}}:163:21 - line:163:21]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:163:24 range=[{{[^:]*}}:163:24 - line:163:24]
// CHECK: (top_level_code_decl range=[{{[^:]*}}:166:1 - line:166:16]
// CHECK:   (brace_stmt range=[{{[^:]*}}:166:1 - line:166:16]
// CHECK:     (call_expr type='()' location={{[^:]*}}:166:1 range=[{{[^:]*}}:166:1 - line:166:16]
// CHECK:       (closure_expr type='() -> ()' location={{[^:]*}}:166:1 range=[{{[^:]*}}:166:1 - line:166:14]
// CHECK:         (parameter_list range=[{{[^:]*}}:166:1 - line:166:1]
// CHECK:         (call_expr type='()' location={{[^:]*}}:166:3 range=[{{[^:]*}}:166:3 - line:166:12]
// CHECK:           (declref_expr type='() -> ()' location={{[^:]*}}:166:3 range=[{{[^:]*}}:166:3 - line:166:3]
// CHECK:           (tuple_expr type='()' location={{[^:]*}}:166:11 range=[{{[^:]*}}:166:11 - line:166:12]
// CHECK:       (tuple_expr type='()' location={{[^:]*}}:166:15 range=[{{[^:]*}}:166:15 - line:166:16]
// CHECK: (func_decl range=[{{[^:]*}}:168:1 - line:177:1]
// CHECK:       (integer_literal_expr type='Int' location={{[^:]*}}:168:32 range=[{{[^:]*}}:168:32 - line:168:32]
// CHECK:       (call_expr type='Int' location={{[^:]*}}:169:32 range=[{{[^:]*}}:169:32 - line:169:48]
// CHECK:         (function_conversion_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:169:32 range=[{{[^:]*}}:169:32 - line:169:42]
// CHECK:           (closure_expr type='(Int, Int) -> Int' location={{[^:]*}}:169:32 range=[{{[^:]*}}:169:32 - line:169:42]
// CHECK:               (parameter "$1" type='Int' interface type='Int') range=[{{[^:]*}}:169:32 - line:169:32]
// CHECK:             (binary_expr type='Int' location={{[^:]*}}:169:37 range=[{{[^:]*}}:169:34 - line:169:39]
// CHECK:               (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:169:37 range=[{{[^:]*}}:169:37 - line:169:37]
// CHECK:                 (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:169:37 range=[{{[^:]*}}:169:37 - line:169:37]
// CHECK:                 (type_expr implicit type='Int.Type' location={{[^:]*}}:169:37 range=[{{[^:]*}}:169:37 - line:169:37]
// CHECK:               (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:169:34 range=[{{[^:]*}}:169:34 - line:169:39]
// CHECK:                 (declref_expr type='Int' location={{[^:]*}}:169:34 range=[{{[^:]*}}:169:34 - line:169:34]
// CHECK:                 (declref_expr type='Int' location={{[^:]*}}:169:39 range=[{{[^:]*}}:169:39 - line:169:39]
// CHECK:         (tuple_expr type='(Int, Int)' location={{[^:]*}}:169:43 range=[{{[^:]*}}:169:43 - line:169:48]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:169:44 range=[{{[^:]*}}:169:44 - line:169:44]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:169:47 range=[{{[^:]*}}:169:47 - line:169:47]
// CHECK:   (brace_stmt range=[{{[^:]*}}:169:51 - line:177:1]
// CHECK:     (func_decl range=[{{[^:]*}}:171:3 - line:173:3]
// CHECK:           (integer_literal_expr type='Int' location={{[^:]*}}:171:35 range=[{{[^:]*}}:171:35 - line:171:35]
// CHECK:           (call_expr type='Int' location={{[^:]*}}:172:35 range=[{{[^:]*}}:172:35 - line:172:51]
// CHECK:             (function_conversion_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:172:35 range=[{{[^:]*}}:172:35 - line:172:45]
// CHECK:               (closure_expr type='(Int, Int) -> Int' location={{[^:]*}}:172:35 range=[{{[^:]*}}:172:35 - line:172:45]
// CHECK:                   (parameter "$1" type='Int' interface type='Int') range=[{{[^:]*}}:172:35 - line:172:35]
// CHECK:                 (binary_expr type='Int' location={{[^:]*}}:172:40 range=[{{[^:]*}}:172:37 - line:172:42]
// CHECK:                   (dot_syntax_call_expr implicit type='(Int, Int) -> Int' location={{[^:]*}}:172:40 range=[{{[^:]*}}:172:40 - line:172:40]
// CHECK:                     (declref_expr type='(Int.Type) -> (Int, Int) -> Int' location={{[^:]*}}:172:40 range=[{{[^:]*}}:172:40 - line:172:40]
// CHECK:                     (type_expr implicit type='Int.Type' location={{[^:]*}}:172:40 range=[{{[^:]*}}:172:40 - line:172:40]
// CHECK:                   (tuple_expr implicit type='(Int, Int)' location={{[^:]*}}:172:37 range=[{{[^:]*}}:172:37 - line:172:42]
// CHECK:                     (declref_expr type='Int' location={{[^:]*}}:172:37 range=[{{[^:]*}}:172:37 - line:172:37]
// CHECK:                     (declref_expr type='Int' location={{[^:]*}}:172:42 range=[{{[^:]*}}:172:42 - line:172:42]
// CHECK:             (tuple_expr type='(Int, Int)' location={{[^:]*}}:172:46 range=[{{[^:]*}}:172:46 - line:172:51]
// CHECK:               (integer_literal_expr type='Int' location={{[^:]*}}:172:47 range=[{{[^:]*}}:172:47 - line:172:47]
// CHECK:               (integer_literal_expr type='Int' location={{[^:]*}}:172:50 range=[{{[^:]*}}:172:50 - line:172:50]
// CHECK:       (brace_stmt range=[{{[^:]*}}:172:54 - line:173:3]
// CHECK:     (pattern_binding_decl range=[{{[^:]*}}:175:3 - line:176:11]
// CHECK:         (declref_expr type='<null>' decl=source_range.(file).defaultArguments(i:j:).i@{{[^:]*}}:168:23 function_ref=unapplied)
// CHECK:             (declref_expr type='<null>' decl=source_range.(file).defaultArguments(i:j:).j@{{[^:]*}}:169:23 function_ref=double)
// CHECK:                   (parameter "$0") range=[{{[^:]*}}:176:3 - line:176:3]
// CHECK:                 (declref_expr type='<null>' decl=source_range.(file).defaultArguments(i:j:).explicit closure discriminator=0.$0@{{[^:]*}}:176:3 function_ref=unapplied))))
// CHECK:       (binary_expr type='<<error type>>' location={{[^:]*}}:175:13 range=[{{[^:]*}}:175:11 - line:176:11]
// CHECK:           (declref_expr type='<null>' decl=source_range.(file).defaultArguments(i:j:).i@{{[^:]*}}:168:23 function_ref=unapplied)
// CHECK:               (declref_expr type='<null>' decl=source_range.(file).defaultArguments(i:j:).j@{{[^:]*}}:169:23 function_ref=double)
// CHECK:                     (parameter "$0") range=[{{[^:]*}}:176:3 - line:176:3]
// CHECK:                   (declref_expr type='<null>' decl=source_range.(file).defaultArguments(i:j:).explicit closure discriminator=0.$0@{{[^:]*}}:176:3 function_ref=unapplied))))
// CHECK:     (var_decl range=[{{[^:]*}}:175:7 - line:175:7]
// CHECK: (struct_decl range=[{{[^:]*}}:179:1 - line:182:1]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:180:3 - line:181:25]
// CHECK:     (tuple_expr type='(Int, Int)' location={{[^:]*}}:180:16 range=[{{[^:]*}}:180:16 - line:180:21]
// CHECK:       (integer_literal_expr type='Int' location={{[^:]*}}:180:17 range=[{{[^:]*}}:180:17 - line:180:17]
// CHECK:       (integer_literal_expr type='Int' location={{[^:]*}}:180:20 range=[{{[^:]*}}:180:20 - line:180:20]
// CHECK:     (tuple_expr type='(Int, Int)' location={{[^:]*}}:180:16 range=[{{[^:]*}}:180:16 - line:180:21]
// CHECK:       (integer_literal_expr type='Int' location={{[^:]*}}:180:17 range=[{{[^:]*}}:180:17 - line:180:17]
// CHECK:       (integer_literal_expr type='Int' location={{[^:]*}}:180:20 range=[{{[^:]*}}:180:20 - line:180:20]
// CHECK:     (tuple_expr type='(Double, Double)' location={{[^:]*}}:181:16 range=[{{[^:]*}}:181:16 - line:181:25]
// CHECK:       (float_literal_expr type='Double' location={{[^:]*}}:181:17 range=[{{[^:]*}}:181:17 - line:181:17]
// CHECK:       (float_literal_expr type='Double' location={{[^:]*}}:181:22 range=[{{[^:]*}}:181:22 - line:181:22]
// CHECK:     (tuple_expr type='(Double, Double)' location={{[^:]*}}:181:16 range=[{{[^:]*}}:181:16 - line:181:25]
// CHECK:       (float_literal_expr type='Double' location={{[^:]*}}:181:17 range=[{{[^:]*}}:181:17 - line:181:17]
// CHECK:       (float_literal_expr type='Double' location={{[^:]*}}:181:22 range=[{{[^:]*}}:181:22 - line:181:22]
// CHECK:   (var_decl range=[{{[^:]*}}:180:8 - line:180:8]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:180:8 - line:180:8]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:180:8 - line:180:8]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:180:8 - line:180:8]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:180:8 - line:180:8]
// CHECK:   (var_decl range=[{{[^:]*}}:180:11 - line:180:11]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:180:11 - line:180:11]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:180:11 - line:180:11]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:180:11 - line:180:11]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:180:11 - line:180:11]
// CHECK:   (var_decl range=[{{[^:]*}}:181:8 - line:181:8]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:181:8 - line:181:8]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:181:8 - line:181:8]
// CHECK:         (parameter "value" interface type='Double') range=[{{[^:]*}}:181:8 - line:181:8]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:181:8 - line:181:8]
// CHECK:   (var_decl range=[{{[^:]*}}:181:11 - line:181:11]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:181:11 - line:181:11]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:181:11 - line:181:11]
// CHECK:         (parameter "value" interface type='Double') range=[{{[^:]*}}:181:11 - line:181:11]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:181:11 - line:181:11]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:179:8 - line:179:8]
// CHECK:       (parameter "d" apiName=d interface type='Double') range=[{{[^:]*}}:179:8 - line:179:8]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:179:8 - line:179:8]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:179:8 - line:179:8]
// CHECK:       (return_stmt range=[{{[^:]*}}:179:8 - line:179:8]
// CHECK: (protocol range=[{{[^:]*}}:184:1 - line:186:1]
// CHECK:   (subscript_decl range=[{{[^:]*}}:185:3 - line:185:43]
// CHECK:     (accessor_decl range=[{{[^:]*}}:185:35 - line:185:35]
// CHECK:         (parameter "native" interface type='Int') range=[{{[^:]*}}:185:12 - line:185:24]
// CHECK:     (accessor_decl range=[{{[^:]*}}:185:39 - line:185:39]
// CHECK:         (parameter "native" interface type='Int') range=[{{[^:]*}}:185:39 - line:185:39]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:185:3 - line:185:3]
// CHECK:         (parameter "native" interface type='Int') range=[{{[^:]*}}:185:13 - line:185:21]
// CHECK: (func_decl range=[{{[^:]*}}:188:1 - line:190:1]
// CHECK:   (parameter_list range=[{{[^:]*}}:188:33 - line:188:34]
// CHECK:   (brace_stmt range=[{{[^:]*}}:188:36 - line:190:1]
// CHECK:     (pattern_binding_decl range=[{{[^:]*}}:189:3 - line:189:16]
// CHECK:     (var_decl range=[{{[^:]*}}:189:7 - line:189:7]
// CHECK:     (var_decl range=[{{[^:]*}}:189:10 - line:189:10]
// CHECK:     (var_decl range=[{{[^:]*}}:189:13 - line:189:13]
// CHECK: (class_decl range=[{{[^:]*}}:192:1 - line:196:1]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:193:3 - line:193:20]
// CHECK:     (integer_literal_expr type='Int' location={{[^:]*}}:193:20 range=[{{[^:]*}}:193:20 - line:193:20]
// CHECK:     (integer_literal_expr type='Int' location={{[^:]*}}:193:20 range=[{{[^:]*}}:193:20 - line:193:20]
// CHECK:   (var_decl range=[{{[^:]*}}:193:7 - line:193:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:193:7 - line:193:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:193:7 - line:193:7]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:193:7 - line:193:7]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:193:7 - line:193:7]
// CHECK:   (pattern_binding_decl range=[{{[^:]*}}:195:8 - line:195:29]
// CHECK:       (declref_expr type='LazyProperties' location={{[^:]*}}:195:24 range=[{{[^:]*}}:195:24 - line:195:24]
// CHECK:     (load_expr implicit type='Int' location={{[^:]*}}:195:29 range=[{{[^:]*}}:195:24 - line:195:29]
// CHECK:       (member_ref_expr type='@lvalue Int' location={{[^:]*}}:195:29 range=[{{[^:]*}}:195:24 - line:195:29]
// CHECK:         (declref_expr type='LazyProperties' location={{[^:]*}}:195:24 range=[{{[^:]*}}:195:24 - line:195:24]
// CHECK:   (var_decl range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:         (parameter "value" interface type='Int') range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:     (accessor_decl implicit range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:   (var_decl implicit range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:   (pattern_binding_decl implicit range=[{{[^:]*}}:195:12 - line:195:12]
// CHECK:   (destructor_decl implicit range=[{{[^:]*}}:192:7 - line:192:7]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:192:7 - line:192:7]
// CHECK:   (constructor_decl implicit range=[{{[^:]*}}:192:7 - line:192:7]
// CHECK:     (brace_stmt implicit range=[{{[^:]*}}:192:7 - line:192:7]
// CHECK:       (return_stmt range=[{{[^:]*}}:192:7 - line:192:7]
// CHECK: (typealias range=[{{[^:]*}}:198:1 - line:199:7]
