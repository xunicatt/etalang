# user config
name = 'gc'
srcs = ['gc.cc']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      lexer_dep,
      ast_dep,
      token_dep,
      object_dep,
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
