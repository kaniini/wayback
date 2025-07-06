scdoc = find_program('scdoc', required: get_option('generate_manpages'))

if scdoc.found()
  sh = find_program('sh')

  manpages = [
    ['wayback-session.scdoc', 'wayback-session.1'],
  ]

  foreach mp : manpages
    custom_target(
      mp[1],
      input: mp[0],
      output: mp[1],
      command: [sh, '-c', '@0@ < @INPUT@ > doc/@1@'.format(scdoc.full_path(), mp[1])],
      install: true,
      install_dir: get_option('mandir') / 'man1',
    )
  endforeach
endif
