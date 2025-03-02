# user config
name = 'repl'
srcs = ['repl.c']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      tokens_dep,
      ast_dep,
      lexer_dep,
      parser_def_dep,
      parser_dep,
      objects_dep,
      scope_dep,
      gc_dep,
      eval_def_dep,
      eval_dep,
      ut_dep,
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
