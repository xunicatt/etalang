# user config
name = 'ast'
srcs = ['ast.cc']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      token_dep,
      lexer_dep,
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
