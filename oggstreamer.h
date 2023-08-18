#define NUM_BUFFERS 4
#define BUFFER_SIZE 65536
#define BUFFER_MSECS 10

typedef struct {
	char *data;
	int data_length;
	int seek_offset;
	
	OggVorbis_File oggStream;
	vorbis_info* vorbisInfo;
	vorbis_comment* vorbisComment;
	
	ALuint buffers[NUM_BUFFERS];
	ALuint source;
	ALenum format;
	
	int open;
	int loop;
	SDL_TimerID timer;
} ogg_stream;
	

int ogg_stream_open(ogg_stream *self, char* path, int loop);
void ogg_stream_release(ogg_stream *self);
int ogg_stream_play(ogg_stream *self);
void ogg_stream_stop(ogg_stream *self);
void ogg_stream_pause(ogg_stream *self);
void ogg_stream_resume(ogg_stream *self);
