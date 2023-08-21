project('CrayonBall', 'c',
	version : '2.3.2',
	default_options: ['b_ndebug=true'],
)

cc = meson.get_compiler('c')
ruby_dir = 'ruby-1.8.7'

exe = executable('aerosol',
	[
		'Aerosol/src/main.c',
		'Aerosol/src/AS_main.c',
		'Aerosol/src/AS_init.c',
		'Aerosol/src/AS_util.c',
		'Aerosol/src/AS_music.c',
		'Aerosol/src/AS_image.c',
		'Aerosol/src/AS_event.c',
		'Aerosol/src/AS_gl.c',
		'Aerosol/src/oggstreamer.c',
		'Aerosol/src/ruby_extras.c',
		
		'Aerosol/src/ogl.c',
		'Aerosol/src/glu.c',
		'Aerosol/src/rbogl.c',
		'Aerosol/src/al.c',
		'Aerosol/src/alc.c',
		'Aerosol/src/rboal.c',
		
		'Aerosol/ext/Chipmunk/src/chipmunk.c',
		'Aerosol/ext/Chipmunk/src/cpSpace.c',
		'Aerosol/ext/Chipmunk/src/cpSpaceStep.c',
		'Aerosol/ext/Chipmunk/src/cpSpaceQuery.c',
		'Aerosol/ext/Chipmunk/src/cpSpaceComponent.c',
		'Aerosol/ext/Chipmunk/src/cpBody.c',
		'Aerosol/ext/Chipmunk/src/cpShape.c',
		'Aerosol/ext/Chipmunk/src/cpPolyShape.c',
		'Aerosol/ext/Chipmunk/src/cpSpaceHash.c',
		'Aerosol/ext/Chipmunk/src/cpArbiter.c',
		'Aerosol/ext/Chipmunk/src/cpCollision.c',
		'Aerosol/ext/Chipmunk/src/cpArray.c',
		'Aerosol/ext/Chipmunk/src/cpHashSet.c',
		'Aerosol/ext/Chipmunk/src/cpVect.c',
		'Aerosol/ext/Chipmunk/src/cpBB.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpConstraint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpGrooveJoint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpRotaryLimitJoint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpDampedRotarySpring.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpPinJoint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpSimpleMotor.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpDampedSpring.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpPivotJoint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpSlideJoint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpGearJoint.c',
		'Aerosol/ext/Chipmunk/src/constraints/cpRatchetJoint.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_chipmunk.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_cpSpace.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_cpVect.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_cpBB.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_cpBody.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_cpShape.c',
		'Aerosol/ext/Chipmunk/ruby/ext/rb_cpConstraint.c',
		
		'Aerosol/ext/Photon/src/photon.c',
		'Aerosol/ext/Photon/src/photon_chipmunk.c',
		'Aerosol/ext/Photon/src/photon_drawspace.c',
		'Aerosol/ext/Photon/src/VectorMath.c',
		'Aerosol/ext/Photon/ruby/ext/rb_photon.c',
		'Aerosol/ext/Photon/ruby/ext/rb_photon_chipmunk.c',
	],
	include_directories: [
		include_directories(ruby_dir),
		include_directories('Aerosol/ext/Chipmunk/include/chipmunk'),
		include_directories('Aerosol/ext/Chipmunk/ruby/ext'),
		include_directories('Aerosol/ext/Photon/src'),
		include_directories('Aerosol/ext/Photon/ruby/ext'),
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
		cc.find_library('ruby-static', dirs: join_paths(meson.current_source_dir(), ruby_dir)),
	],
	c_args: [
		'-Wno-unused-parameter',
		'-Wno-comment',
		'-Wno-unused-but-set-variable',
		'-Wno-maybe-uninitialized',
		'-Wno-unused-const-variable',
		'-Wno-switch-unreachable',
		'-Wno-implicit-function-declaration',
		'-Wno-unused-function',
		'-Wno-pedantic',
		'-Wno-unknown-pragmas',
		'-Wno-parentheses',
		'-Wno-sign-compare',
		'-Wno-cast-function-type',
		'-Wno-deprecated',
		'-Wno-missing-field-initializers',
		'-Wno-char-subscripts',
	],
	install: true,
	install_dir: 'share/games/CrayonBall/bin',
)

# Is there a better way to make a launch script?
custom_target(
	'crayonball',
	capture: true,
	output: 'crayonball',
	command: [
		'echo',
		'#! /usr/bin/env sh\n',
		'cd ' + get_option('prefix') + '/share/games/CrayonBall\n',
		'bin/aerosol\n',
	],
	depends: [exe],
	install: true,
	install_dir: 'bin',
	install_mode: 'rwxr-xr-x',
)

install_data(
	'com.howlingmoonsoftware.CrayonBall.desktop',
	install_dir: 'share/applications',
)

install_data(
	'com.howlingmoonsoftware.CrayonBall.png',
	install_dir: 'share/icons/hicolor/512x512/apps',
)

install_data(
	'com.howlingmoonsoftware.CrayonBall.metainfo.xml',
	install_dir: 'share/metainfo',
)

install_subdir('Aerosol/ruby', install_dir: 'share/games/CrayonBall')
install_subdir('CrayonBall/src', install_dir: 'share/games/CrayonBall')
install_subdir('CrayonBall/gfx', install_dir: 'share/games/CrayonBall')
install_subdir('CrayonBall/sfx', install_dir: 'share/games/CrayonBall')

test('basic', exe)
