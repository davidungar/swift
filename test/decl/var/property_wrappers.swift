// RUN: %target-typecheck-verify-swift -swift-version 5


// SR-11060

class SR_11060_Class {
  @SR_11060_Wrapper var property: Int = 1234 // expected-error {{missing argument for parameter 'string' in property wrapper initializer; add 'wrappedValue' and 'string' arguments in '@SR_11060_Wrapper(...)'}}
}

@propertyWrapper
struct SR_11060_Wrapper {
  var wrappedValue: Int
  
  init(wrappedValue: Int, string: String) { // expected-note {{'init(wrappedValue:string:)' declared here}}
    self.wrappedValue = wrappedValue
  }
}

