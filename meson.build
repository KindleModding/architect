project('architect', 'c', version: 'v1.0.0', meson_version: '>=1.1')

sources = files(
    'architect.c'
)

include_dirs = include_directories(
  '.'
)

executable('architect', sources, include_directories: include_dirs, cpp_args: ['-static', '-static-libgcc', '-static-libstdc++'], link_args: ['-static', '-static-libgcc', '-static-libstdc++'])