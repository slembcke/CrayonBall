
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
		
		'ext/Chipmunk/src/chipmunk.c',
		'ext/Chipmunk/src/cpSpace.c', 'ext/Chipmunk/src/cpSpaceStep.c',
		'ext/Chipmunk/src/cpSpaceQuery.c', 'ext/Chipmunk/src/cpSpaceComponent.c',
		'ext/Chipmunk/src/cpBody.c', 'ext/Chipmunk/src/cpShape.c',
		'ext/Chipmunk/src/cpPolyShape.c', 'ext/Chipmunk/src/cpSpaceHash.c',
		'ext/Chipmunk/src/cpArbiter.c', 'ext/Chipmunk/src/cpCollision.c',
		'ext/Chipmunk/src/cpArray.c', 'ext/Chipmunk/src/cpHashSet.c',
		'ext/Chipmunk/src/cpVect.c', 'ext/Chipmunk/src/cpBB.c',
		'ext/Chipmunk/src/constraints/cpConstraint.c', 'ext/Chipmunk/src/constraints/cpGrooveJoint.c',
		'ext/Chipmunk/src/constraints/cpRotaryLimitJoint.c', 'ext/Chipmunk/src/constraints/cpDampedRotarySpring.c',
		'ext/Chipmunk/src/constraints/cpPinJoint.c', 'ext/Chipmunk/src/constraints/cpSimpleMotor.c',
		'ext/Chipmunk/src/constraints/cpDampedSpring.c', 'ext/Chipmunk/src/constraints/cpPivotJoint.c',
		'ext/Chipmunk/src/constraints/cpSlideJoint.c', 'ext/Chipmunk/src/constraints/cpGearJoint.c',
		'ext/Chipmunk/src/constraints/cpRatchetJoint.c',
		
		'ext/Chipmunk/ruby/ext/rb_chipmunk.c', 'ext/Chipmunk/ruby/ext/rb_cpSpace.c',
		'ext/Chipmunk/ruby/ext/rb_cpVect.c', 'ext/Chipmunk/ruby/ext/rb_cpBB.c',
		'ext/Chipmunk/ruby/ext/rb_cpBody.c', 'ext/Chipmunk/ruby/ext/rb_cpShape.c',
		'ext/Chipmunk/ruby/ext/rb_cpConstraint.c',
		
		'ext/Photon/src/photon.c', 'ext/Photon/src/photon_chipmunk.c',
		'ext/Photon/src/photon_drawspace.c', 'ext/Photon/src/VectorMath.c',
		'ext/Photon/ruby/ext/rb_photon.c', 'ext/Photon/ruby/ext/rb_photon_chipmunk.c',
	],
	include_directories: [
		include_directories(ruby_dir),
		include_directories('ext/Chipmunk/include/chipmunk'),
		include_directories('ext/Chipmunk/ruby/ext'),
		include_directories('ext/Photon/src'),
		include_directories('ext/Photon/ruby/ext'),
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
		'-Wno-unused-parameter', '-Wno-comment', '-Wno-unused-but-set-variable', '-Wno-maybe-uninitialized', '-Wno-unused-const-variable',
		'-Wno-switch-unreachable', '-Wno-implicit-function-declaration', '-Wno-unused-function', '-Wno-pedantic',
		'-Wno-unknown-pragmas', '-Wno-parentheses', '-Wno-sign-compare', '-Wno-cast-function-type', '-Wno-deprecated',
		'-Wno-missing-field-initializers', '-Wno-char-subscripts',
	],
)

test('basic', exe)
