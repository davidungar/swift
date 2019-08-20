// RUN: %target-swift-ide-test -syntax-coloring -source-filename %s | %FileCheck %s
// RUN: %target-swift-ide-test -syntax-coloring -typecheck -source-filename %s | %FileCheck %s



func f(x: Int) -> Int {

 



  // CHECK: <str>"""
  // CHECK-NEXT: This is a multiline</str>\<anchor>(</anchor> <str>"interpolated"</str> <anchor>)</anchor><str>string
  // CHECK-NEXT: </str>\<anchor>(</anchor>
  // CHECK-NEXT: <str>"""
  // CHECK-NEXT: inner
  // CHECK-NEXT: """</str>
  // CHECK-NEXT: <anchor>)</anchor><str>
  // CHECK-NEXT: """</str>
  """
      This is a multiline\( "interpolated" )string
   \(
   """
    inner
   """
   )
   """


}

// CHECK: <kw>func</kw> bar(x: <type>Int</type>) -> (<type>Int</type>, <type>Float</type>) {
func bar(x: Int) -> (Int, Float) {
  // CHECK: foo({{(<type>)?}}Float{{(</type>)?}}())
  foo(Float())
}
