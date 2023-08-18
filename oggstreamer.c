#include <stdlib.h>
#include <string.h>

#if defined(__APPLE_CC__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>
#include <SDL2/SDL.h>

#include "oggstreamer.h"

static void
al_error(ogg_stream *self, int line)
{
	int error = alGetError();

	switch(error){
		case AL_INVALID_NAME:
			printf("AL: INVALID_NAME (%d)\n", line);
			break;
		case AL_INVALID_ENUM:
			printf("AL: INVALID_ENUM (%d)\n", line);
			break;
		case AL_INVALID_VALUE:
			printf("AL: INVALID_VALUE (%d)\n", line);
			break;
		case AL_INVALID_OPERATION:
			printf("AL: INVALID_OPERATION (%d)\n", line);
			break;
		case AL_OUT_OF_MEMORY:
			printf("AL: OUT_OF_MEMORY (%d)\n", line);
			break;
	}
}

static char*
ogg_error(int code)
{
	switch(code)
	{
		case OV_EREAD:
			return "Ogg: Read from media.";
		case OV_ENOTVORBIS:
			return "Ogg: Not Vorbis data.";
		case OV_EVERSION:
			return "Ogg: Vorbis version mismatch.";
		case OV_EBADHEADER:
			return "Ogg: Invalid Vorbis header.";
		case OV_EFAULT:
			return "Ogg: Internal logic fault (bug or heap/stack corruption.";
		default:
			return "Ogg: Unknown Ogg error.";
	}
}

static size_t
cb_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	ogg_stream *self = (ogg_stream*)datasource;
	
	int bytes = size*nmemb;
	int remain = self->data_length - self->seek_offset - bytes;
	if(remain < 0) bytes += remain;
	
	memcpy(ptr, self->data + self->seek_offset, bytes);
	self->seek_offset += bytes;
	
	
	return bytes;
}

static int
cb_seek(void *datasource, ogg_int64_t offset, int whence)
{
	ogg_stream *self = (ogg_stream*)datasource;
	int seek_offset;
	
	switch(whence){
	case SEEK_CUR:
		seek_offset = self->seek_offset + offset;
		break;
	case SEEK_END:
		seek_offset = self->data_length + offset;
		break;
	case SEEK_SET:
		seek_offset = offset;
		break;
	default:
		printf("Invalid whence enumerant\n");
		exit(1);
	}
	
	if(seek_offset > self->data_length) return -1;
	
	self->seek_offset = seek_offset;
	return 0;	
}

static int
cb_close(void *datasource)
{
	ogg_stream *self = (ogg_stream*)datasource;
	free(self->data);
	self->data = 0;
	return 0;
}

static long
cb_tell(void *datasource)
{
	ogg_stream *self = (ogg_stream*)datasource;
	return self->seek_offset;
}


static int
stream_playing(ogg_stream *self)
{
	ALenum state;
	alGetSourcei(self->source, AL_SOURCE_STATE, &state);
	
	return (state == AL_PLAYING);
}

static int
stream_stopped(ogg_stream *self)
{
	ALenum state;
	alGetSourcei(self->source, AL_SOURCE_STATE, &state);
	
	return (state == AL_STOPPED || state == AL_INITIAL);
}

static int
stream_paused(ogg_stream *self)
{
	ALenum state;
	alGetSourcei(self->source, AL_SOURCE_STATE, &state);
	
	return (state == AL_PAUSED);
}

static int
stream_read(ogg_stream *self, ALuint buffer)
{
	char pcm[BUFFER_SIZE];
	int  size = 0;
	int  section;
	int  result;

	while(size < BUFFER_SIZE)
	{
		result = ov_read(&self->oggStream, pcm + size, BUFFER_SIZE - size,
			(SDL_BYTEORDER==SDL_BIG_ENDIAN), 2, 1, &section);
		
		if(result==0 && self->loop)
		ov_raw_seek(&self->oggStream, 0);
	
		if(result > 0)
			size += result;
		else
			if(result < 0){
				printf("Ogg error: %s\n", ogg_error(result));
			} else {
				break;
			}
	}
    
	if(size == 0){
		printf("stream end\n");
		return 0;
	}
  
	alBufferData(buffer, self->format, pcm, size, self->vorbisInfo->rate);
	al_error(self, __LINE__);
	
	return 1;
}

static int
stream_update(ogg_stream *self)
{
	int processed;
	int active = 1;

	alGetSourcei(self->source, AL_BUFFERS_PROCESSED, &processed);
	if(processed==NUM_BUFFERS)
		printf("All music buffers used up\n");

	while(processed--)
	{
		ALuint buffer;
		
		alSourceUnqueueBuffers(self->source, 1, &buffer);
		al_error(self, __LINE__);

		active = stream_read(self, buffer);
		if(active)
			alSourceQueueBuffers(self->source, 1, &buffer);
	}

	al_error(self, __LINE__);

	return active;
}

static void
stream_empty(ogg_stream *self)
{
	int queued;
	
	alGetSourcei(self->source, AL_BUFFERS_QUEUED, &queued);
	
	while(queued--)
	{
		ALuint buffer;

		alSourceUnqueueBuffers(self->source, 1, &buffer);
		al_error(self, __LINE__);
	}
}

static Uint32
timerCallback(Uint32 interval, void *param)
{
	ogg_stream *ogg = param;
	// check/queue more buffers
	int active = stream_update(ogg);
	
	if(!stream_playing(ogg))
	{
		if(!ogg_stream_play(ogg))
			printf("Ogg abruptly stopped.\n");
		else
			printf("Ogg stream was interrupted. Stopping music.\n");
		
		SDL_RemoveTimer(ogg->timer);
	}
	
	return active ? interval : 0;
}


/*
 * API functions
 */

int ogg_stream_open(ogg_stream *self, char* path, int loop)
{
	FILE* oggFile = fopen(path, "rb");
    
	if(!oggFile){
			printf("Could not open Ogg file.\n");
			exit(1);
	}

	int err;
	self->seek_offset = 0;
	fseek(oggFile, 0, SEEK_END);
	self->data_length = ftell(oggFile);
	fseek(oggFile, 0, SEEK_SET);
	
	self->data = malloc(self->data_length);
	
	fread(self->data, sizeof(char), self->data_length, oggFile);
	fclose(oggFile);
	
	ov_callbacks cbacks = {
		.read_func = cb_read,
		.seek_func = cb_seek,
		.close_func = cb_close,
		.tell_func = cb_tell,
	};
	
	if(ov_open_callbacks(self, &self->oggStream, 0, 0, cbacks)){
		printf("ov_open_callbacks() failed %s\n", path);
		printf("%s\n", ogg_error(err));
		exit(1);
	}

	self->vorbisInfo = ov_info(&self->oggStream, -1);
	self->vorbisComment = ov_comment(&self->oggStream, -1);

	if(self->vorbisInfo->channels == 1)
			self->format = AL_FORMAT_MONO16;
	else
			self->format = AL_FORMAT_STEREO16;
			
			
	alGenBuffers(NUM_BUFFERS, self->buffers);
	al_error(self, __LINE__);
	alGenSources(1, &self->source);
	al_error(self, __LINE__);
	
	self->loop = loop;
    
	alSource3f(self->source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(self->source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(self->source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (self->source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (self->source, AL_SOURCE_RELATIVE, AL_TRUE      );
	
	return 1;
}

void ogg_stream_release(ogg_stream *self)
{
	if(alcGetCurrentContext()!=NULL){
		ogg_stream_stop(self);
		stream_empty(self);
		alDeleteSources(1, &self->source);
		al_error(self, __LINE__);
		alDeleteBuffers(NUM_BUFFERS, self->buffers);
		al_error(self, __LINE__);
	}

	ov_clear(&self->oggStream);
}

int ogg_stream_play(ogg_stream *self)
{
	if(!stream_stopped(self)) return 1;
        
	int processed;
	alGetSourcei(self->source, AL_BUFFERS_PROCESSED, &processed);
	
	ALuint trash[processed];
	alSourceUnqueueBuffers(self->source, processed, trash);
	
	int num_buffers = NUM_BUFFERS;
	for(int i=0; i<NUM_BUFFERS; i++){
		if(!stream_read(self, self->buffers[i])){
			num_buffers = i;
			break;
		}
	}
	
	alSourceQueueBuffers(self->source, num_buffers, self->buffers);
	alSourcePlay(self->source);
	
	self->timer = SDL_AddTimer(BUFFER_MSECS, timerCallback, self);
    
	return 1;
}

void ogg_stream_stop(ogg_stream *self)
{
	if(stream_stopped(self)) return;
	
	SDL_RemoveTimer(self->timer);
	alSourceStop(self->source);
	
	ov_raw_seek(&self->oggStream, 0);
}

void ogg_stream_pause(ogg_stream *self)
{
	if(!stream_playing(self)) return;
	
	alSourcePause(self->source);
	SDL_RemoveTimer(self->timer);
}

void ogg_stream_resume(ogg_stream *self)
{
	if(!stream_paused(self)) return;
	
	stream_update(self);
	self->timer = SDL_AddTimer(BUFFER_MSECS, timerCallback, self);
	
	alSourcePlay(self->source);
}
