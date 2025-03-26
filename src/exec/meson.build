# user config
name = 'exec'
srcs = ['exec.cc']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      token_dep,
      lexer_dep,
      ast_dep,
      parser_dep,
      object_dep,
      gc_dep,
      scope_dep,
      eval_dep,
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
