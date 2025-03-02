# user config
name = 'eval_core'
srcs = [
  'builtinfn.c',
  'statements.c',
  'expressions.c',
]

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      tokens_dep,
      lexer_dep,
      ast_dep,
      scope_dep,
      objects_dep,
      gc_dep,
      ut_dep,
      eval_builtins_dep,
      eval_def_dep,
    ],
  ),
)
set_variable(
  name + '_dep',
  declare_dependency(
    link_with: get_variable(name + '_lib'),
    include_directories: include_directories('.'),
  ),
)
