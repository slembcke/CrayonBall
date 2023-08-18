
project('CrayonBall', 'c',
	version : '0.1',
#	default_options : ['warning_level=3']
)

#add_global_arguments('-Wno-unused-parameter', language : 'c')

cc = meson.get_compiler('c')
ruby_dir = 'ruby-1.8.7-p352'

exe = executable('crayonball',
	[
		'main.c',
		'AS_main.c',
		'AS_util.c',
		'AS_image.c',
		'AS_event.c',
		'AS_gl.c',
		'oggstreamer.c',
		'AS_music.c',
		'ruby_extras.c',
		'ogl.c', 'rbogl.c',
		'al.c', 'rboal.c',
	],
	include_directories: [
		include_directories(ruby_dir)
	],
	dependencies: [
		cc.find_library('m'),
		cc.find_library('crypt'),
		cc.find_library('SDL2'),
		cc.find_library('png'),
		cc.find_library('ogg'),
		cc.find_library('vorbis'),
		cc.find_library('OpenGL'),
#		cc.find_library('OpenAL-soft'),
		cc.find_library('ruby-static', dirs: meson.current_source_dir() + '/' + ruby_dir),
	],
	c_args: [
		'-Wno-unused-parameter', '-Wno-comment', '-Wno-unused-but-set-variable', '-Wno-maybe-uninitialized', '-Wno-switch-unreachable', '-Wno-implicit-function-declaration',
	],
)

test('basic', exe)
