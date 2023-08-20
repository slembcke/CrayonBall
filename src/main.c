#include <stdio.h>

#include <SDL2/SDL.h>
#include <ruby.h>

#include "AS.h"

extern void Init_opengl(void);
extern void Init_cp(void);
extern void Init_openal(void);
extern void Init_myAdditions(void);
extern void Init_chipmunk(void);
extern void Init_photonEXT(void);

extern void showRegistrationDialog(void);

int main(int argc, char *argv[])
{
//	showRegistrationDialog();
	
  // TODO: use argv to set things like starting path, maybe even a starting gamestate for a game. 
  // Maybe we can eliminate the init.rb file with this.
  
	//Initialize the Ruby interpereter.
	ruby_init(); 
	ruby_script("Aerosol"); // Sets the name of the script
	ruby_init_loadpath(); // Initialize $: (load path for the rest of the program)
	
	// push the CLI arguments into Ruby. Can't use ruby_options(), it parses them as the ruby command.
	VALUE args = rb_ary_new();
	for(int i=1; i<argc; i++)
		rb_ary_push(args, rb_str_new2(argv[i]));
	
	rb_funcall(rb_eval_string("ARGV"), rb_intern("replace"), 1, args);
	
  // load the init.rb file, if we can find it
	AS_init();
	
	Init_opengl();
	Init_openal();
	Init_myAdditions();
	Init_chipmunk();
	Init_photonEXT();
	
	int err;
	rb_protect(Start_Aerosol, 0, &err);
	if(err) AS_printException("Error starting Aerosol.");
	
	//not safe!
//	rb_require("as_start.rb");
	
	rb_exit(0);
	
	return(0);
}
