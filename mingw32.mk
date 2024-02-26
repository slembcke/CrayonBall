# mingw cross compile notes
# meson setup --cross-file mingw-cross.toml build-mingw
# meson compile -C build-mingw

# Binaries 
# https://github.com/oneclick/rubyinstaller/releases/download/ruby-1.8.7-p374/ruby-1.8.7-p374-i386-mingw32.7z

# headers:
# https://cache.ruby-lang.org/pub/ruby/1.8/ruby-1.8.7.tar.gz
# ./configure --host=i686-w64-mingw32
# remove timezone from from win32.h


CC = i686-w64-mingw32-gcc

CFLAGS += -I /tmp/ruby-1.8.7
CFLAGS += -I Aerosol/ext/Chipmunk/include/chipmunk
CFLAGS += -I Aerosol/ext/Chipmunk/ruby/ext
CFLAGS += -I Aerosol/ext/Photon/src
CFLAGS += -I Aerosol/ext/Photon/ruby/ext
CFLAGS += $(shell mingw32-pkg-config --cflags sdl2 ogg vorbis vorbisfile openal libpng)

LDFLAGS += $(shell mingw32-pkg-config --libs sdl2 ogg vorbis vorbisfile openal libpng)
LDFLAGS += ruby-1.8.7-p374-i386-mingw32/lib/libmsvcrt-ruby18-static.a
LDFLAGS += -l opengl32 -l glu32
LDFLAGS += -l ws2_32

SRC += Aerosol/src/main.c
SRC += Aerosol/src/AS_main.c
SRC += Aerosol/src/AS_init.c
SRC += Aerosol/src/AS_util.c
SRC += Aerosol/src/AS_music.c
SRC += Aerosol/src/AS_image.c
SRC += Aerosol/src/AS_event.c
SRC += Aerosol/src/AS_gl.c
SRC += Aerosol/src/oggstreamer.c
SRC += Aerosol/src/ruby_extras.c
SRC += Aerosol/src/ogl.c
SRC += Aerosol/src/glu.c
SRC += Aerosol/src/rbogl.c
SRC += Aerosol/src/al.c
SRC += Aerosol/src/alc.c
SRC += Aerosol/src/rboal.c
SRC += Aerosol/ext/Chipmunk/src/chipmunk.c
SRC += Aerosol/ext/Chipmunk/src/cpSpace.c
SRC += Aerosol/ext/Chipmunk/src/cpSpaceStep.c
SRC += Aerosol/ext/Chipmunk/src/cpSpaceQuery.c
SRC += Aerosol/ext/Chipmunk/src/cpSpaceComponent.c
SRC += Aerosol/ext/Chipmunk/src/cpBody.c
SRC += Aerosol/ext/Chipmunk/src/cpShape.c
SRC += Aerosol/ext/Chipmunk/src/cpPolyShape.c
SRC += Aerosol/ext/Chipmunk/src/cpSpaceHash.c
SRC += Aerosol/ext/Chipmunk/src/cpArbiter.c
SRC += Aerosol/ext/Chipmunk/src/cpCollision.c
SRC += Aerosol/ext/Chipmunk/src/cpArray.c
SRC += Aerosol/ext/Chipmunk/src/cpHashSet.c
SRC += Aerosol/ext/Chipmunk/src/cpVect.c
SRC += Aerosol/ext/Chipmunk/src/cpBB.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpConstraint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpGrooveJoint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpRotaryLimitJoint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpDampedRotarySpring.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpPinJoint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpSimpleMotor.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpDampedSpring.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpPivotJoint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpSlideJoint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpGearJoint.c
SRC += Aerosol/ext/Chipmunk/src/constraints/cpRatchetJoint.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_chipmunk.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_cpSpace.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_cpVect.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_cpBB.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_cpBody.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_cpShape.c
SRC += Aerosol/ext/Chipmunk/ruby/ext/rb_cpConstraint.c
SRC += Aerosol/ext/Photon/src/photon.c
SRC += Aerosol/ext/Photon/src/photon_chipmunk.c
SRC += Aerosol/ext/Photon/src/photon_drawspace.c
SRC += Aerosol/ext/Photon/src/VectorMath.c
SRC += Aerosol/ext/Photon/ruby/ext/rb_photon.c
SRC += Aerosol/ext/Photon/ruby/ext/rb_photon_chipmunk.c

aerosol.exe: $(SRC:.c=.o)
	$(CC) $^ $(LDFLAGS) -o $@

# Eh... this doesn't work, dunno what's up with that URL
# ruby-1.8.7-p374-i386-mingw32.7z:
# 	curl -O https://github.com/oneclick/rubyinstaller/releases/download/ruby-1.8.7-p374/ruby-1.8.7-p374-i386-mingw32.7z

%: %.7z
	7z x $<

clean:
	-rm $(SRC:.c=.o)
