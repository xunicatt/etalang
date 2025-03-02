# user config
name = 'eval_def'

set_variable(
  name + '_dep',
  declare_dependency(
    include_directories: include_directories('.'),
    dependencies: [
      tokens_dep,
      lexer_dep,
      objects_dep,
      scope_dep,
    ],
  ),
)
