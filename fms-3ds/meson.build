smdh_app_name = 'Fit Meter Sync'
smdh_app_desc = 'Synchronise your Wii Fit U meter with your 3DS!'
smdh_app_author = 'Chris Josten'
smdh_app_icon = '/opt/devkitpro/libctru/default_icon.png'
app_name = 'fitmetersync'


fms3ds_inc = [
]

fms3ds_files = [
	'src/link.cpp',
	'src/main.cpp',
	'src/reserver.cpp',
	'src/tsdeque.cpp'
]

fms3ds_exe = executable('fms-3ds',
	fms3ds_files,
	include_directories: fms3ds_inc,
	dependencies: libfms_dep
)

smdhtool_exe = find_program('smdhtool')
xdsxtool_exe = find_program('3dsxtool')

fms3ds_smdh = custom_target(
	'smdh',
	build_by_default: true,
	input: [smdh_app_icon],
	output: 'fitmetersync.smdh',
	command: [smdhtool_exe, '--create', smdh_app_name, smdh_app_desc, smdh_app_author, '@INPUT@', '@OUTPUT@']
)

fitmetersync_3dsx = custom_target(
	'3dsx',
	build_by_default: true,
	input: [fms3ds_exe, fms3ds_smdh],
	output: app_name + '.3dsx',
	command: [xdsxtool_exe, '@INPUT0@', '@OUTPUT0@', '--smdh=@INPUT1@'])
