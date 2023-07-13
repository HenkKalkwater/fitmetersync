boost_dep = dependency('boost', modules: ['program_options'])

fmscli_files = [
	'src/command_common.cpp',
	'src/command_dump.cpp',
	'src/command_retrieve.cpp',
	'src/main.cpp'
]

fmscli_exe = executable(
	'fms-cli',
	fmscli_files,
	dependencies: [libfms_dep, boost_dep]
)
