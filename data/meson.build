desktop_conf = configuration_data()
desktop_conf.set('bindir', pkg_bindir)

desktop_file = i18n.merge_file(
  input:  configure_file(
    input: 'org.kop316.phoshantispam.desktop.in.in',
    output: 'org.kop316.phoshantispam.desktop.in',
    configuration: desktop_conf,
  ),
  output: 'org.kop316.phoshantispam.desktop',
  type: 'desktop',
  po_dir: pkg_po_dir,
  install: true,
  install_dir: join_paths(get_option('sysconfdir'), 'xdg/autostart')
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
       args: [desktop_file])
endif
