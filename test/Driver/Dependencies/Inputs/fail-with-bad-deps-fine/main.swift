# Fine-grained v0
---
allNodes:
  - key:
      kind:            sourceFileProvide
      aspect:          interface
      context:         ''
      name:            './depends-on-main.swiftdeps'
    fingerprint:     after
    sequenceNumber:  0
    defsIDependUpon: [ 6, 5, 2, 4 ]
    isProvides:      true
  - key:
      kind:            sourceFileProvide
      aspect:          implementation
      context:         ''
      name:            './depends-on-main.swiftdeps'
    fingerprint:     after
    sequenceNumber:  1
    defsIDependUpon: [  ]
    isProvides:      true
  - key:
      kind:            topLevel
      aspect:          interface
      context:         ''
      name:            M
    sequenceNumber:  2
    defsIDependUpon: [ 0 ]
    isProvides:      true
  - key:
      kind:            topLevel
      aspect:          implementation
      context:         ''
      name:            M
    sequenceNumber:  3
    defsIDependUpon: [  ]
    isProvides:      true
  - key:
      kind:            topLevel
      aspect:          interface
      context:         ''
      name:            main
    sequenceNumber:  4
    defsIDependUpon: [  ]
    isProvides:      false
  - key:
      kind:            externalDepend
      aspect:          interface
      context:         ''
      name:            '/Users/ungar/s/fine2/build/Ninja-DebugAssert/swift-macosx-x86_64/lib/swift/macosx/Swift.swiftmodule/x86_64.swiftmodule'
    sequenceNumber:  5
    defsIDependUpon: [  ]
    isProvides:      false
  - key:
      kind:            externalDepend
      aspect:          interface
      context:         ''
      name:            '/Users/ungar/s/fine2/build/Ninja-DebugAssert/swift-macosx-x86_64/lib/swift/macosx/SwiftOnoneSupport.swiftmodule/x86_64.swiftmodule'
    sequenceNumber:  6
    defsIDependUpon: [  ]
    isProvides:      false
...
