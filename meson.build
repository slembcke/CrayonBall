
project('CrayonBall', 'c',
	version : '0.1',
#	default_options : ['warning_level=3']
)

#add_global_arguments('-Wno-unused-parameter', language : 'c')

cc = meson.get_compiler('c')
ruby_dir = 'ruby-1.8.7-p352'

exe = executable('crayonball',
	[
		'src/main.c',
		'src/AS_main.c',
		'src/AS_init.c',
		'src/AS_util.c',
		'src/AS_music.c',
		'src/AS_image.c',
		'src/AS_event.c',
		'src/AS_gl.c',
		'src/oggstreamer.c',
		'src/ruby_extras.c',
		'src/ogl.c', 'src/glu.c', 'src/rbogl.c',
		'src/al.c', 'src/alc.c', 'src/rboal.c',
	],
	include_directories: [
		include_directories(ruby_dir)
	],
	dependencies: [
		cc.find_library('m'),
		cc.find_library('crypt'),
		cc.find_library('SDL2'),
		cc.find_library('png'),
		cc.find_library('vorbisfile'),
		cc.find_library('OpenGL'),
		cc.find_library('GLU'),
		cc.find_library('openal'),
		cc.find_library('ruby-static', dirs: meson.current_source_dir() + '/' + ruby_dir),
	],
	c_args: [
		'-Wno-unused-parameter', '-Wno-comment', '-Wno-unused-but-set-variable', '-Wno-maybe-uninitialized', '-Wno-switch-unreachable', '-Wno-implicit-function-declaration',
	],
)

test('basic', exe)
