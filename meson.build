project(
  'eta',
  'c',
  version: 'v0.0.2',
  default_options: [
    'c_std=c11',
    'c_flags=-Wall -Wextra -Werror',
  ],
)

if get_option('buildtype') == 'debug'
  add_project_arguments('-DLEXER_DEBUG_ENABLE', language: 'c')
  add_project_arguments('-DPARSER_DEBUG_ENABLE', language: 'c')
  add_project_arguments('-DGC_DEBUG_ENABLE', language: 'c')
endif

subdir('src/ut')
subdir('src/tokens')
subdir('src/lexer')
subdir('src/ast')
subdir('src/parser')
subdir('src/objects')
subdir('src/scope')
subdir('src/gc')
subdir('src/eval')
subdir('src/repl')
subdir('src/exec')

executable(
  meson.project_name(),
  'main.c',
  install: true,
  dependencies: [
    repl_dep,
    exec_dep,
  ],
  link_args: ['-lm'],
)
