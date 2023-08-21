#include <ruby.h>
#include <SDL2/SDL.h>

#include "AS.h"

static VALUE rbas_mEvent;

static VALUE rbas_cKeyboardEvent;
static VALUE rbas_cMouseMotionEvent;
static VALUE rbas_cMouseButtonEvent;

static int mx, my, mrx, mry;
static int mbuttons[255];

typedef struct{
	int buttons, axes, hats;
	int *buttonState, *hatState;
	float *axisState;
} AS_jstick;

#define MAX_JSTICKS 5

//static int numSticks;
//static AS_jstick *jsticks;

static void do_key(SDL_Event event){
	unsigned mod = event.key.keysym.mod;
	
	VALUE modifiers = rb_ary_new();
	if(mod & KMOD_LSHIFT) rb_ary_push(modifiers, ID2SYM(rb_intern("LSHIFT")));
	if(mod & KMOD_RSHIFT) rb_ary_push(modifiers, ID2SYM(rb_intern("RSHIFT")));
	if(mod & KMOD_LCTRL	) rb_ary_push(modifiers, ID2SYM(rb_intern("LCTRL")));
	if(mod & KMOD_RCTRL	) rb_ary_push(modifiers, ID2SYM(rb_intern("RCTRL")));
	if(mod & KMOD_LALT	) rb_ary_push(modifiers, ID2SYM(rb_intern("LALT")));
	if(mod & KMOD_RALT	) rb_ary_push(modifiers, ID2SYM(rb_intern("RALT")));
	if(mod & KMOD_LGUI	) rb_ary_push(modifiers, ID2SYM(rb_intern("LMETA")));
	if(mod & KMOD_RGUI	) rb_ary_push(modifiers, ID2SYM(rb_intern("RMETA")));
	if(mod & KMOD_NUM		) rb_ary_push(modifiers, ID2SYM(rb_intern("NUM")));
	if(mod & KMOD_CAPS	) rb_ary_push(modifiers, ID2SYM(rb_intern("CAPS")));
	if(mod & KMOD_MODE	) rb_ary_push(modifiers, ID2SYM(rb_intern("MODE")));
	if(mod & KMOD_CTRL	) rb_ary_push(modifiers, ID2SYM(rb_intern("CRTL")));
	if(mod & KMOD_SHIFT	) rb_ary_push(modifiers, ID2SYM(rb_intern("SHIFT")));
	if(mod & KMOD_ALT		) rb_ary_push(modifiers, ID2SYM(rb_intern("ALT")));
	if(mod & KMOD_GUI	) rb_ary_push(modifiers, ID2SYM(rb_intern("META")));
	
	VALUE argv[] = {
		event.key.state ? Qtrue : Qfalse,
		INT2FIX(event.key.keysym.sym),
		modifiers
	};
	rb_yield(rb_class_new_instance(3, argv, rbas_cKeyboardEvent));
}

static void do_mouse_button(SDL_Event event){
	VALUE argv[] = {
		event.button.state ? Qtrue : Qfalse,
		INT2FIX(event.button.button),
		rb_ary_new3(2,INT2FIX(event.motion.x),INT2FIX(AS_SCREEN_H - event.motion.y - 1) )
	};
	rb_yield(rb_class_new_instance(3, argv, rbas_cMouseButtonEvent));
}

static void do_mouse_motion(SDL_Event event){
	VALUE argv[] = {
		rb_ary_new3(2,INT2FIX(event.motion.x),INT2FIX(AS_SCREEN_H - event.motion.y - 1) ),
		rb_ary_new3(2,INT2FIX(event.motion.xrel),INT2FIX(-event.motion.yrel) ),
	};
	rb_yield(rb_class_new_instance(2, argv, rbas_cMouseMotionEvent));
}

VALUE rbas_getEvents(VALUE self){
	SDL_Event event;
	int block = rb_block_given_p();
	
	while(SDL_PollEvent(&event)){
		int eventid = -1;
		VALUE eventvalue = Qnil;
		
		switch(event.type){
			case SDL_QUIT:
				printf("SDL_QUIT event caught.");
				rb_exit(0);
				break;
				
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				do_key(event);
				break;
				
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				do_mouse_button(event);
				break;
				
			case SDL_MOUSEMOTION:
				do_mouse_motion(event);
				break;	
						
//			case SDL_JOYBUTTONDOWN:
//			case SDL_JOYBUTTONUP:
//				jsticks[event.jbutton.which].buttonState[event.jbutton.button] = event.button.state;
//				eventid = event.jbutton.button<<4 | (3 + event.jbutton.which);
//				eventvalue = event.button.state ? Qtrue : Qfalse;
//				break;
//				
//			case SDL_JOYAXISMOTION:
//				jsticks[event.jaxis.which].axisState[event.jaxis.axis] = event.jaxis.value/32768.0;
//				eventid = event.jaxis.axis<<4 | (7 + event.jaxis.which);
//				eventvalue = rb_float_new(event.jaxis.value/32768.0);
//				break;
//				
//			case SDL_JOYHATMOTION:
//				jsticks[event.jhat.which].hatState[event.jhat.hat] = event.jhat.value;
//				eventid = event.jhat.hat<<4 | (11 + event.jhat.which);
//				eventvalue = INT2NUM(event.jhat.value);
//				break;
//				
			default:
				//printf("unhandled event\n");
				break;
		}
		
		if(eventid != -1 && block) rb_yield(rb_ary_new3(2, INT2FIX(eventid), eventvalue));
	}
	
	return Qnil;
}
/*
VALUE rbas_getKeystate(VALUE self, VALUE keysym){
	int sym = FIX2INT(keysym);
	if(sym <0 || sym >= SDLK_LAST) rb_raise(rb_eArgError, "No such key #%d.", sym);
	return keys[sym] ? Qtrue : Qfalse;
}

VALUE rbas_getButtonstate(VALUE self, VALUE num){
	int numint = FIX2INT(num);
	if(numint < 0 || numint >= 5){
		rb_raise(rb_eArgError, "No such mouse button %d.", numint);
	}
	return keys[numint] ? Qtrue : Qfalse;
}

VALUE rbas_getJoyButtonState(VALUE self, VALUE joy, VALUE num){
	int joyint = FIX2INT(joy); int numint = FIX2INT(num);
	if(joyint < 0 || numint < 0 || joyint >= numSticks || numint >= jsticks[joyint].buttons){
		rb_raise(rb_eArgError, "No such button or joystick %d:%d.", joyint, numint);
	}
	return jsticks[joyint].buttonState[numint] ? Qtrue : Qfalse;
}

VALUE rbas_getJoyAxisState(VALUE self, VALUE joy, VALUE num){
	int joyint = FIX2INT(joy); int numint = FIX2INT(num);
	if(joyint < 0 || numint < 0 || joyint >= numSticks || numint >= jsticks[joyint].axes){
		rb_raise(rb_eArgError, "No such axis or joystick %d:%d.", joyint, numint);
	}
	return rb_float_new(jsticks[joyint].axisState[numint]);
}

VALUE rbas_getJoyHatState(int argc, VALUE *argv, VALUE self){
	if(argc == 2 || argc == 3){
		int joyint = FIX2INT(argv[0]); int numint = FIX2INT(argv[1]);
		if(joyint < 0 || numint < 0 || joyint >= numSticks || numint >= jsticks[joyint].hats){
			rb_raise(rb_eArgError, "No such hat or joystick %d:%d.", joyint, numint);
		}
		if(argc == 2) return INT2FIX(jsticks[joyint].hatState[numint]);
		return INT2FIX( jsticks[joyint].hatState[numint] | FIX2INT(argv[2]) );
	}
	
	printf("wrong number of arguments for AS::Event.get_hat_value");
	rb_exit(1);
	return Qnil;
}

VALUE rbas_getMouse(VALUE self)
{
	return rb_ary_new3(2, INT2NUM(mx), INT2NUM(my));
}

VALUE rbas_getMouseRel(VALUE self)
{
	return rb_ary_new3(2, INT2NUM(mrx), INT2NUM(mry));
}


void set_up_joysticks(void)
{
	numSticks = SDL_NumJoysticks();
	numSticks = (numSticks <= MAX_JSTICKS) ? numSticks : MAX_JSTICKS;
	printf("%d Joysticks found (max 4)\n", numSticks);
	
	VALUE sJoystick = rb_eval_string("AS::ImageStruct = Struct.new(:data, :w, :h, :format)");
	volatile VALUE joyinfo = rb_ary_new();
	
	jsticks = calloc(numSticks, sizeof(AS_jstick));
	
	int i;
	for(i = 0; i < numSticks; i++){
		SDL_Joystick *sdl_stick = SDL_JoystickOpen(i);
		int buttons = SDL_JoystickNumButtons(sdl_stick);
		int axes = SDL_JoystickNumAxes(sdl_stick);
		int hats = SDL_JoystickNumHats(sdl_stick);
			
		AS_jstick *jstick = &jsticks[i];
		jstick->buttons = buttons;
		jstick->axes = axes;
		jstick->hats = hats;
		jstick->buttonState = calloc(buttons, sizeof(int));
		jstick->axisState = calloc(axes, sizeof(float));
		jstick->hatState = calloc(hats, sizeof(int));
		
		volatile VALUE name = rb_str_new2(SDL_JoystickName(i));
		volatile VALUE rb_stick = rb_funcall(sJoystick, rb_intern("new"), 5,
								 INT2NUM(i), name, INT2NUM(buttons), INT2NUM(axes), INT2NUM(hats));
		rb_ary_push(joyinfo, rb_stick);
	}
	
	rb_define_const(rbas_mEvent, "Joysticks", joyinfo);
}
*/
void Init_ASevents(void){
	rbas_mEvent = rb_define_module_under(rbas_mAS, "Event");
	
	rb_define_module_function(rbas_mEvent, "each", &rbas_getEvents, 0);
	rb_define_module_function(rbas_mEvent, "dequeue_events", &rbas_getEvents, 0);	
	/*
	rb_define_module_function(rbas_mEvent, "key_value", &rbas_getKeystate, 1);
	rb_define_module_function(rbas_mEvent, "mbutton_value", &rbas_getButtonstate, 1);
	rb_define_module_function(rbas_mEvent, "jbutton_value", &rbas_getJoyButtonState, 2);
	rb_define_module_function(rbas_mEvent, "jaxis_value", &rbas_getJoyAxisState, 2);
	rb_define_module_function(rbas_mEvent, "jhat_value", &rbas_getJoyHatState, -1);
	rb_define_module_function(rbas_mEvent, "mouse_pos", &rbas_getMouse, 0);
	rb_define_module_function(rbas_mEvent, "mouse_rel", &rbas_getMouseRel, 0);
	*/
	rbas_cKeyboardEvent = rb_eval_string("AS::Event::KeyboardEvent");
	rbas_cMouseMotionEvent = rb_eval_string("AS::Event::MouseMotionEvent");
	rbas_cMouseButtonEvent = rb_eval_string("AS::Event::MouseButtonEvent");
	
	for(int i=0; i<5; i++) mbuttons[i]=0;
	
	mx = 0; mrx = 0;
	my = 0; mry = 0;
	
//	set_up_joysticks();
}
