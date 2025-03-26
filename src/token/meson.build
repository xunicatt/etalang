# user config
name = 'token'
srcs = ['token.cc']

# presets
set_variable(
  name + '_lib',
  static_library(
    name,
    srcs,
    dependencies: [],
  ),
)
set_variable(
  name + '_dep',
  declare_dependency(
    link_with: get_variable(name + '_lib'),
    include_directories: include_directories('.'),
  ),
)
