subdir('src/parser_def')
subdir('src/infixfns')
subdir('src/prefixfns')
subdir('src/precedences')
subdir('src/core')

# user config
name = 'parser'
srcs = ['parser.c']

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
      parser_map_infixfns_dep,
      parser_map_prefixfns_dep,
      parser_def_dep,
      parser_map_precedences_dep,
      parser_core_dep,
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
