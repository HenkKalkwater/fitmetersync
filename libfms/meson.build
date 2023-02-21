libfms_files = [
	'src/core/crc.cpp',
	'src/platform/iradaptermanager.cpp',
	'src/platform/lock.cpp',
	'src/protocol/link.cpp',
	'src/pcap.cpp'
]

if target_machine.system() == 'horizon'
	libfms_files += [
		'src/platform/ctru/condvar.cpp',
		'src/platform/ctru/iradapter.cpp',
		'src/platform/ctru/iradaptermanager.cpp',
		'src/platform/ctru/mutex.cpp',
		'src/platform/ctru/thread.cpp'
	]
elif target_machine.system() == 'linux'
	libfms_files += [
		'src/platform/linux/condvar.cpp',
		'src/platform/linux/iradapter.cpp',
		'src/platform/linux/iradaptermanager.cpp',
		'src/platform/linux/mutex.cpp',
		'src/platform/linux/thread.cpp'
	]
endif

libfms_inc = [
	include_directories('src')
]

libfms = static_library(
	'fms',
	libfms_files,
	include_directories: libfms_inc
)

libfms_dep = declare_dependency(
	link_with: libfms,
	include_directories: libfms_inc
)
