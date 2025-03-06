subdir('src/eval_def')
subdir('src/builtins')
subdir('src/core')

# user config
name = 'eval'
srcs = [
  'eval.c',
]

libffi_dep = dependency('libffi', fallback: ['libffi'])

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      libffi_dep,
      tokens_dep,
      lexer_dep,
      ast_dep,
      scope_dep,
      objects_dep,
      gc_dep,
      ut_dep,
      eval_def_dep,
      eval_builtins_dep,
      eval_core_dep,
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
