project('fitmetersync', 'cpp')

src_files = [
	'src/platform/lock.cpp',
	'src/crc.cpp',
	'src/link.cpp',
	'src/main.cpp',
	'src/pcap.cpp',
	'src/reserver.cpp',
	'src/tsdeque.cpp'
]

if target_machine.system() == 'horizon'
	src_files += [
		'src/platform/ctru/condvar.cpp',
		'src/platform/ctru/mutex.cpp',
		'src/platform/ctru/thread.cpp'
	]
endif

executable('fitmetersync',
	src_files
)
