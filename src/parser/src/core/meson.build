# user config
name = 'parser_core'
srcs = ['statements.c', 'expressions.c']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      ut_dep,
      tokens_dep,
      lexer_dep,
      ast_dep,
      parser_def_dep,
      parser_map_prefixfns_dep,
      parser_map_infixfns_dep,
      parser_map_precedences_dep,
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
