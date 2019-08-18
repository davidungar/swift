// Check source locations (only InterpolatedStringLiteral for now).

func string_interpolation() {
  "\("abc")"
  <#Int#>
}

// RUN: not %target-swift-frontend -dump-ast %s | %FileCheck %s
// CHECK: (interpolated_string_literal_expr {{.*}} end_loc_for_name_lookup=SOURCE_DIR{{/|\\}}test{{/|\\}}Parse{{/|\\}}source_locs.swift:4:13 {{.*}}
// CHECK: (editor_placeholder_expr type='()' {{.*}} trailing_angle_bracket_loc=SOURCE_DIR{{/|\\}}test{{/|\\}}Parse{{/|\\}}source_locs.swift:5:9

