project('phoshspamblock', 'c',
        version: '1.0',
  meson_version: '>= 0.40.0',
)


config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
configure_file(
  output: 'phoshspamblock-config.h',
  configuration: config_h,
)
add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

pkg_srcdir    = meson.source_root()
pkg_builddir  = meson.build_root()
pkg_prefix    = get_option('prefix')
pkg_datadir   = join_paths(pkg_prefix, get_option('datadir'))
pkg_bindir    = join_paths(pkg_prefix, get_option('bindir'))
i18n  = import('i18n')
pkg_po_dir    = join_paths(pkg_srcdir, 'po')

subdir('src')
subdir('data')

