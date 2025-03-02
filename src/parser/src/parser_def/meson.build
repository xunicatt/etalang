name = 'parser_def'

set_variable(
  name + '_dep',
  declare_dependency(
    include_directories: include_directories('.'),
    dependencies: [
      ut_dep,
      tokens_dep,
      lexer_dep,
    ],
  ),
)
