project('fitmetersync', 'cpp',
	default_options: ['cpp_std=c++17'])


subdir('libfms')
subdir('fms-cli')

if target_machine.system() == 'horizon'
	subdir('fms-3ds')
endif
