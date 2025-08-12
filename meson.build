project('architect', 'cpp', version: 'v1.0.0', default_options: ['cpp_std=c++17'], meson_version: '>=1.1')

sources = files(
    'architect.cpp'
)

include_dirs = include_directories(
  '.'
)

executable('architect', sources, include_directories: include_dirs, cpp_args: ['-static', '-static-libgcc', '-static-libstdc++'], link_args: ['-static', '-static-libgcc', '-static-libstdc++'])