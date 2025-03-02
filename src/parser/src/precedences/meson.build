# user config
name = 'parser_map_precedences'
srcs = ['precedences.c']

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
