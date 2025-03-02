# user config
subdir('src/specialchars')
subdir('src/keywords')

name = 'lexer'
srcs = ['lexer.c']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [
      ut_dep,
      tokens_dep,
      lexer_map_keywords_dep,
      lexer_map_specialchars_dep,
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
