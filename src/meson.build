phoshspamblock_sources = [
  'main.c',
  'spamblock-client.c',
]

phoshspamblock_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0', version: '>= 2.50')
]

executable('phoshspamblock', phoshspamblock_sources,
  dependencies: phoshspamblock_deps,
  install: true,
)
