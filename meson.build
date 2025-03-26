project(
  'eta',
  'cpp',
  version: 'v0.0.1',
  default_options: [
    'cpp_std=c++23',
    'cpp_flags=-Wall -Wextra -Werror',
  ],
)

if get_option('buildtype') == 'debug'
  add_project_arguments('-DTOKENS_DEBUG_INFO_ENABLE', language: 'cpp')
  add_project_arguments('-DAST_DEBUG_INFO_ENABLE', language: 'cpp')
  add_project_arguments('-DGC_DEBUG_INFO_ENABLE', language: 'cpp')
endif

subdir('src/token')
subdir('src/lexer')
subdir('src/ast')
subdir('src/parser')
subdir('src/object')
subdir('src/gc')
subdir('src/scope')
subdir('src/eval')
subdir('src/repl')
subdir('src/exec')

executable(
  meson.project_name(),
  'main.cc',
  install: true,
  dependencies: [
    repl_dep,
    exec_dep,
  ],
)
