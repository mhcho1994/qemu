t = []

python3 = import('python').find_installation('python3')
python_dep = python3.dependency()

cc = meson.get_compiler('c')
python_versions = ['python3.15', 'python3.14', 'python3.13', 'python3.12', 'python3.11', 'python3.10']
libpython = not_found

foreach v : python_versions
  libpython = cc.find_library(v, required: false)
  if libpython.found()
    break
  endif
endforeach

if not libpython.found()
  error('Required Python library not found (3.10 - 3.15)')
endif

if get_option('plugins')
  foreach i : ['bb', 'empty', 'inline', 'insn', 'mem', 'reset', 'syscall', 'virtual']
    if host_os == 'windows'
      t += shared_module(i, files(i + '.c') + '../../../contrib/plugins/win32_linker.c',
                         include_directories: '../../../include/qemu',
                         link_depends: [win32_qemu_plugin_api_lib],
                         link_args: win32_qemu_plugin_api_link_flags,
                         dependencies: glib)
    else
      t += shared_module(i, files(i + '.c'),
                         include_directories: '../../../include/qemu',
                         dependencies: [glib, python_dep, libpython])
    endif
  endforeach
endif

if t.length() > 0
  alias_target('test-plugins', t)
else
  run_target('test-plugins', command: find_program('true'))
endif

plugin_modules += t
