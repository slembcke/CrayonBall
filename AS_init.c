#include <stdio.h>

#include <ruby.h>
#include <SDL2/SDL.h>

#if defined(__APPLE_CC__)
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#elif defined(WIN32)
  #include <AL/al.h>
  #include <AL/alc.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

#include "AS.h"

ALCdevice *AL_device;
ALCcontext *AL_context;

static VALUE load_init_dot_rb(VALUE unused)
{
	//Initialize the Aerosol module.
	rbas_mAS = rb_define_module("AS");
	
	//attempt to load aerosol.dat otherwise load init.rb
	FILE *dat_file = fopen("main_logo.png", "rb");
	if(dat_file){
		unsigned char key[1024];
		fseek(dat_file, 32412, SEEK_SET); // throw away, fake header
		fread(key, 1, 1024, dat_file);
		
		VALUE serialized_hash = rb_str_new(NULL, 0);
		char buff[1024];
		int length = 0;
		
		while(length = fread(buff, 1, 1024, dat_file)){
			for(int i=0; i<length; i++)
				buff[i] ^= key[i];
			
			rb_str_cat(serialized_hash, buff, length);
		}
		
		rb_gv_set("serialized_hash", serialized_hash);
		rb_eval_string(
			"DEBUG_MODE = false \n"
			"$code_hash = Marshal.load($serialized_hash) \n"
			"$serialized_hash = nil \n"
			
			"def hash_require(path) \n"
			"  key = nil \n"
			
			"  if $code_hash.key?(path) \n"
			"    key = path \n"
			"  elsif $code_hash.key?(path + '.rb') \n"
			"    key = path + '.rb' \n"
			"  end \n"
			
			"  return true if $\".include?(key) \n"
			
			"  if key \n"
			"    $\" << key \n"
			"    #puts \"key -> #{key}\" \n"
			"    eval($code_hash[key], binding, key) \n"
			
			"    return true \n"
			"  else \n"
			"    #puts \"can't load -> #{path}\" \n"
			"    return false \n"
			"  end \n"
			"end \n"
			
			"def require(path) \n"
			"  if hash_require(path) \n"
			"    return true \n"
			"  else \n"
			"    Kernel.require(path) \n"
			"  end \n"
			"end \n"
		);
	} else {
		rb_eval_string(
			//Evaluate init.rb if it exists.
			"puts \"Starting Aerosol in #{Dir.pwd}\" \n"
			"load 'init.rb' if File.exist?('init.rb') \n"
		);
	}
	
	rb_eval_string(
		//Define AS::GAME_MAIN and AS::LIB_DIR if they are undefined. This will run the code inside the bundle.
		"AS::LIB_DIR = File.expand_path '.' unless defined?(AS::LIB_DIR) \n"
		//Add the Aerosol directory to the search path
		"$: << AS::LIB_DIR \n"
	);
	
	return Qnil; //need to return something.
}

static void initialize_SDL(void)
{
	Uint32 initflags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |SDL_INIT_TIMER;
	
	//Initialize the SDL library
	if ( SDL_Init(initflags) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
}

static void initialize_AL(void)
{
	AL_device = alcOpenDevice(NULL);
	AL_context = alcCreateContext(AL_device, NULL);
	alcMakeContextCurrent(AL_context);
}	

void AS_init(void)
{
	initialize_SDL();
	initialize_AL();

	int err = 0;
	rb_protect(load_init_dot_rb, 0, &err);
	if(err) AS_printException("Exception during initializiation. Check init.rb for errors.");
}
