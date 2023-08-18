#include <stdlib.h>

#include <ruby.h>
#include <SDL2/SDL.h>

#ifdef __APPLE__
  #include <OpenAL/al.h>
  #include <Ogg/ogg.h>
  #include <Vorbis/vorbisfile.h>
#else
  #include <AL/al.h>
  #include <ogg/ogg.h>
  #include <vorbis/vorbisfile.h>
#endif

#include "oggstreamer.h"
#include "AS.h"


VALUE rbas_cMusic;
VALUE rbas_soundStruct;


void rbas_music_free(ogg_stream *oggs)
{
	if(oggs->open) ogg_stream_release(oggs);
	free(oggs);
}
	
VALUE rbas_music_alloc(VALUE klass)
{
	ogg_stream *oggs = malloc(sizeof(ogg_stream));
	oggs->open = 0;
	
	return Data_Wrap_Struct(klass, 0, rbas_music_free, oggs);
}

VALUE rbas_music_init(VALUE self, VALUE filename, VALUE loop)
{
	ogg_stream *oggs;
	Data_Get_Struct(self, ogg_stream, oggs);
	
	ogg_stream_open(oggs, StringValuePtr(filename), loop);
	
	return self;
}

VALUE rbas_music_play(VALUE self)
{
	ogg_stream *oggs;
	Data_Get_Struct(self, ogg_stream, oggs);
	
	ogg_stream_play(oggs);
	
	return Qnil;
}

VALUE rbas_music_stop(VALUE self)
{
	ogg_stream *oggs;
	Data_Get_Struct(self, ogg_stream, oggs);
	
	ogg_stream_stop(oggs);
	
	return Qnil;
}

VALUE rbas_music_pause(VALUE self)
{
	ogg_stream *oggs;
	Data_Get_Struct(self, ogg_stream, oggs);
	
	ogg_stream_pause(oggs);
	
	return Qnil;
}

VALUE rbas_music_resume(VALUE self)
{
	ogg_stream *oggs;
	Data_Get_Struct(self, ogg_stream, oggs);
	
	ogg_stream_resume(oggs);
	
	return Qnil;
}

VALUE rbas_music_volume(VALUE self, VALUE volume)
{
	ogg_stream *oggs;
	Data_Get_Struct(self, ogg_stream, oggs);
	
	alSourcef(oggs->source, AL_GAIN, NUM2DBL(volume));
	
	return volume;
}


static VALUE
rbas_load_vorbis(VALUE self, VALUE filename)
{
	FILE *file = fopen(StringValuePtr(filename), "rb");
	if(!file) rb_raise(rb_eIOError, "File not found: %s", filename);
	
	OggVorbis_File vorbis;
	if(ov_open(file, &vorbis, NULL, 0))
		rb_raise(rb_eIOError, "Could not read %s", filename);
	
	vorbis_info *info = ov_info(&vorbis, -1);
	int samples = ov_pcm_total(&vorbis, -1);
	int rate = info->rate;
	int channels = info->channels;
	
	// Breaks for non mono/stereo
	ALenum ALformat = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	int bytes = 2*samples*channels;
	
	VALUE data = rb_str_new(NULL, 0);
	rb_str_resize(data, bytes);
	
	int bytes_read = 0;
	while(bytes_read < bytes){
		int remain = bytes - bytes_read;
		char *cursor = StringValuePtr(data) + bytes_read;
		
		long value = ov_read(&vorbis, cursor, remain, (SDL_BYTEORDER==SDL_BIG_ENDIAN), 2, 1, NULL);
		if(value < 0) rb_raise(rb_eIOError, "Could not read %s", filename);
		
		bytes_read += value;
	}
	
	ov_clear(&vorbis);
	return rb_struct_new(rb_eval_string("AS::SoundStruct"), data, INT2NUM(rate), INT2NUM(ALformat));
}

void Init_ASmusic(void)
{
	rbas_cMusic = rb_define_class_under(rbas_mAS, "Music", rb_cObject);
	rb_define_alloc_func(rbas_cMusic, rbas_music_alloc);
	rb_define_method(rbas_cMusic, "initialize", rbas_music_init, 2);
	rb_define_method(rbas_cMusic, "play", rbas_music_play, 0);
	rb_define_method(rbas_cMusic, "stop", rbas_music_stop, 0);
	rb_define_method(rbas_cMusic, "pause", rbas_music_pause, 0);
	rb_define_method(rbas_cMusic, "resume", rbas_music_resume, 0);
	rb_define_method(rbas_cMusic, "volume=", rbas_music_volume, 1);
	
	rb_define_module_function(rbas_mAS, "load_vorbis", rbas_load_vorbis, 1);
}
