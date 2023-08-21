#if defined(__APPLE_CC__)
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif
#include <ruby.h>
#include <intern.h>
#include "rboal.h"

static VALUE al_Enable(VALUE obj, VALUE arg_capability)
{
    ALenum capability = (ALenum)NUM2INT(arg_capability);
    alEnable(capability);
    return Qnil;
}

static VALUE al_Disable(VALUE obj, VALUE arg_capability)
{
    ALenum capability = (ALenum)NUM2INT(arg_capability);
    alDisable(capability);
    return Qnil;
}

static VALUE al_IsEnabled(VALUE obj, VALUE arg_capability)
{
    ALenum capability = (ALenum)NUM2INT(arg_capability);
    ALboolean result = alIsEnabled(capability);
    return result ? Qtrue : Qfalse;
}

/*
static VALUE al_Hint(VALUE obj, VALUE arg_target, VALUE arg_mode)
{
    ALenum target = (ALenum)NUM2INT(arg_target);
    ALenum mode = (ALenum)NUM2INT(mode);
    alHint(target, mode);
    return Qnil;
}
*/

/*
static VALUE al_GetBooleanv(VALUE obj, VALUE arg_param);
static VALUE al_GetIntegerv(VALUE obj, VALUE arg_param);
static VALUE al_GetFloatv(VALUE obj, VALUE arg_param);
static VALUE al_GetDoublev(VALUE obj, VALUE arg_param);
*/

static VALUE al_GetString(VALUE obj, VALUE arg_param)
{
    ALenum param = (ALenum)NUM2INT(arg_param);
    const ALchar *result = alGetString(param);
    return rb_str_new2((const char *)result);
}

static VALUE al_GetBoolean(VALUE obj, VALUE arg_param)
{
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALboolean result = alGetBoolean(param);
    return result ? Qtrue : Qfalse;
}

static VALUE al_GetInteger(VALUE obj, VALUE arg_param)
{
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALint result = alGetInteger(param);
    return INT2NUM(result);
}

#define al_GetFloat al_GetDouble

static VALUE al_GetDouble(VALUE obj, VALUE arg_param)
{
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALdouble result = alGetDouble(param);
    return rb_float_new(result);
}

static VALUE al_GetError(VALUE obj)
{
    ALenum result = alGetError();
    return INT2FIX(result);
}

/*
static VALUE al_IsExtensionPresent(VALUE obj, VALUE arg_fname);
static VALUE al_GetProcAddress(VALUE obj, VALUE arg_fname);
static VALUE al_GetEnumValue(VALUE obj, VALUE arg_ename);
*/

static VALUE al_Listenerf(VALUE obj, VALUE arg_pname, VALUE arg_param)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALfloat param = (ALfloat)NUM2DBL(arg_param);
    alListenerf(pname, param);
    return Qnil;
}

static VALUE al_Listeneri(VALUE obj, VALUE arg_pname, VALUE arg_param)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALint param = (ALint)NUM2INT(arg_param);
    alListeneri(pname, param);
    return Qnil;
}

static VALUE al_Listener3f(VALUE obj, VALUE arg_pname, VALUE arg_f1, VALUE arg_f2, VALUE arg_f3)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALfloat f1 = (ALfloat)NUM2DBL(arg_f1);
    ALfloat f2 = (ALfloat)NUM2DBL(arg_f2);
    ALfloat f3 = (ALfloat)NUM2DBL(arg_f3);
    alListener3f(pname, f1, f2, f3);
    return Qnil;
}

static VALUE al_Listenerfv(VALUE obj, VALUE arg_pname, VALUE arg_param)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    unsigned int param_count = rb_ary_size(arg_param);
    ALfloat *param = alloca(sizeof(ALfloat) * param_count);
    unsigned int param_index;
    for (param_index = 0; param_index < param_count; ++param_index)
    {
        param[param_index] = (ALfloat)NUM2DBL(rb_ary_entry(arg_param, param_index));
    }
    alListenerfv(pname, param);
    return Qnil;
}

static VALUE al_GetListeneri(VALUE obj, VALUE arg_pname)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALint value;
    alGetListeneri(pname, &value);
    return INT2NUM(value);
}

static VALUE al_GetListenerf(VALUE obj, VALUE arg_pname)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALfloat value;
    alGetListenerf(pname, &value);
    return rb_float_new(value);
}

/*
static VALUE al_GetListeneriv(VALUE obj, VALUE arg_pname);
static VALUE al_GetListenerfv(VALUE obj, VALUE arg_pname);
*/

static VALUE al_GetListener3f(VALUE obj, VALUE arg_pname)
{
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALfloat f1, f2, f3;
    alGetListener3f(pname, &f1, &f2, &f3);
    VALUE result = rb_ary_new();
    rb_ary_push(result, rb_float_new(f1));
    rb_ary_push(result, rb_float_new(f2));
    rb_ary_push(result, rb_float_new(f3));
    return result;    
}

static VALUE al_GenSources(VALUE obj, VALUE arg_n)
{
    ALsizei n = (ALsizei)NUM2INT(arg_n);
    ALuint *sources = alloca(sizeof(ALuint) * n);
    alGenSources(n, sources);
    VALUE result = rb_ary_new();
    ALsizei result_index;
    for (result_index = 0; result_index < n; ++result_index)
    {
        rb_ary_push(result, INT2NUM(sources[result_index]));
    }
    return result;
}

static VALUE al_DeleteSources(VALUE obj, VALUE arg_sources)
{
    unsigned int sources_count = rb_ary_size(arg_sources);
    ALuint *sources = alloca(sizeof(ALuint) * sources_count);
    unsigned int sources_index;
    for (sources_index = 0; sources_index < sources_count; ++sources_index)
    {
        sources[sources_index] = (ALuint)NUM2INT(rb_ary_entry(arg_sources, sources_index));
    }
    alDeleteSources(sources_count, sources);
    return Qnil;
}

static VALUE al_IsSource(VALUE obj, VALUE arg_sid)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALboolean result = alIsSource(sid);
    return result ? Qtrue : Qfalse;
}

static VALUE al_Sourcei(VALUE obj, VALUE arg_sid, VALUE arg_param, VALUE arg_value)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALint value = (ALint)NUM2INT(arg_value);
    alSourcei(sid, param, value);
    return Qnil;    
}

static VALUE al_Sourcef(VALUE obj, VALUE arg_sid, VALUE arg_param, VALUE arg_value)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALfloat value = (ALfloat)NUM2DBL(arg_value);
    alSourcef(sid, param, value);
    return Qnil;    
}

static VALUE al_Source3f(VALUE obj, VALUE arg_sid, VALUE arg_param, VALUE arg_f1, VALUE arg_f2, VALUE arg_f3)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALfloat f1 = (ALfloat)NUM2DBL(arg_f1);
    ALfloat f2 = (ALfloat)NUM2DBL(arg_f2);
    ALfloat f3 = (ALfloat)NUM2DBL(arg_f3);
    alSource3f(sid, param, f1, f2, f3);
    return Qnil;    
}

static VALUE al_Sourcefv(VALUE obj, VALUE arg_sid, VALUE arg_param, VALUE arg_values)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum param = (ALenum)NUM2INT(arg_param);
    unsigned int values_count = rb_ary_size(arg_values);
    ALfloat *values = alloca(sizeof(ALfloat) * values_count);
    unsigned int values_index;
    for (values_index = 0; values_index < values_count; ++values_index)
    {
        values[values_index] = (ALfloat)NUM2DBL(rb_ary_entry(arg_values, values_index));
    }
    alSourcefv(sid, param, values);
    return Qnil;    
}

static VALUE al_GetSourcei(VALUE obj, VALUE arg_sid, VALUE arg_pname)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALint result;
    alGetSourcei(sid, pname, &result);
    return INT2NUM(result);    
}

/*
static VALUE al_GetSourceiv(VALUE obj, VALUE arg_sid, VALUE arg_pname);
*/

static VALUE al_GetSourcef(VALUE obj, VALUE arg_sid, VALUE arg_pname)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum pname = (ALenum)NUM2INT(arg_pname);
    ALfloat result;
    alGetSourcef(sid, pname, &result);
    return rb_float_new(result);    
}

/*
static VALUE al_GetSourcefv(VALUE obj, VALUE arg_sid, VALUE arg_pname);
*/

static VALUE al_SourcePlay(VALUE obj, VALUE arg_sid)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    alSourcePlay(sid);
    return Qnil;    
}

static VALUE al_SourcePause(VALUE obj, VALUE arg_sid)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    alSourcePause(sid);
    return Qnil;    
}

static VALUE al_SourceRewind(VALUE obj, VALUE arg_sid)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    alSourceRewind(sid);
    return Qnil;    
}

static VALUE al_SourceStop(VALUE obj, VALUE arg_sid)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    alSourceStop(sid);
    return Qnil;    
}

static VALUE al_GenBuffers(VALUE obj, VALUE arg_n)
{
    ALsizei n = (ALsizei)NUM2INT(arg_n);
    ALuint *buffers = alloca(sizeof(ALuint) * n);
    alGenBuffers(n, buffers);
    VALUE result = rb_ary_new();
    ALsizei result_index;
    for (result_index = 0; result_index < n; ++result_index)
    {
        rb_ary_push(result, INT2NUM(buffers[result_index]));
    }
    return result;
}

static VALUE al_DeleteBuffers(VALUE obj, VALUE arg_buffers)
{
    unsigned int buffers_count = rb_ary_size(arg_buffers);
    ALuint *buffers = alloca(sizeof(ALuint) * buffers_count);
    unsigned int buffers_index;
    for (buffers_index = 0; buffers_index < buffers_count; ++buffers_index)
    {
        buffers[buffers_index] = (ALuint)NUM2INT(rb_ary_entry(arg_buffers, buffers_index));
    }
    alDeleteSources(buffers_count, buffers);
    return Qnil;
}

static VALUE al_IsBuffer(VALUE obj, VALUE arg_buffer)
{
    ALuint buffer = (ALuint)NUM2INT(arg_buffer);
    ALboolean result = alIsBuffer(buffer);
    return result ? Qtrue : Qfalse;
}

static VALUE al_BufferData(VALUE obj, VALUE arg_buffer, VALUE arg_format, VALUE arg_data, VALUE arg_size, VALUE arg_freq)
{
    ALuint buffer = (ALuint)NUM2INT(arg_buffer);
    ALenum format = (ALuint)NUM2INT(arg_format);
    ALvoid *data = (ALvoid *)STR2CSTR(arg_data);
    ALsizei size = (ALsizei)NUM2INT(arg_size);
    ALsizei freq = (ALsizei)NUM2INT(arg_freq);
    alBufferData(buffer, format, data, size, freq);
    return Qnil;
}

static VALUE al_GetBufferi(VALUE obj, VALUE arg_buffer, VALUE arg_param)
{
    ALuint buffer = (ALuint)NUM2INT(arg_buffer);
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALint result;
    alGetBufferi(buffer, param, &result);
    return INT2NUM(result);    
}

static VALUE al_GetBufferf(VALUE obj, VALUE arg_buffer, VALUE arg_param)
{
    ALuint buffer = (ALuint)NUM2INT(arg_buffer);
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALfloat result;
    alGetBufferf(buffer, param, &result);
    return rb_float_new(result);    
}

/*
static VALUE al_GetBufferiv(VALUE obj, VALUE arg_buffer, VALUE arg_param);
static VALUE al_GetBufferfv(VALUE obj, VALUE arg_buffer, VALUE arg_param);
*/

static VALUE al_SourceQueueBuffers(VALUE obj, VALUE arg_sid, VALUE arg_bids)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    unsigned int bids_count = rb_ary_size(arg_bids);
    ALuint *bids = alloca(sizeof(ALuint) * bids_count);
    unsigned int bids_index;
    for (bids_index = 0; bids_index < bids_count; ++bids_index)
    {
        bids[bids_index] = (ALuint)NUM2INT(rb_ary_entry(arg_bids, bids_index));
    }
    alSourceQueueBuffers(sid, bids_count, bids);
    return Qnil;
}

static VALUE al_SourceUnqueueBuffers(VALUE obj, VALUE arg_sid, VALUE arg_bids)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    unsigned int bids_count = rb_ary_size(arg_bids);
    ALuint *bids = alloca(sizeof(ALuint) * bids_count);
    unsigned int bids_index;
    for (bids_index = 0; bids_index < bids_count; ++bids_index)
    {
        bids[bids_index] = (ALuint)NUM2INT(rb_ary_entry(arg_bids, bids_index));
    }
    alSourceUnqueueBuffers(sid, bids_count, bids);
    return Qnil;
}

/*
static VALUE al_Queuei(VALUE obj, VALUE arg_sid, VALUE arg_param, VALUE arg_value)
{
    ALuint sid = (ALuint)NUM2INT(arg_sid);
    ALenum param = (ALenum)NUM2INT(arg_param);
    ALint value = (ALint)NUM2INT(arg_value);
    alQueuei(sid, param, value);
    return Qnil;    
}
*/

static VALUE al_DopplerFactor(VALUE obj, VALUE arg_value)
{
    ALfloat value = (ALfloat)NUM2DBL(arg_value);
    alDopplerFactor(value);
    return Qnil;
}

static VALUE al_DopplerVelocity(VALUE obj, VALUE arg_value)
{
    ALfloat value = (ALfloat)NUM2DBL(arg_value);
    alDopplerVelocity(value);
    return Qnil;
}

static VALUE al_DistanceModel(VALUE obj, VALUE arg_distanceModel)
{
    ALenum distanceModel = (ALenum)NUM2INT(arg_distanceModel);
    alDistanceModel(distanceModel);
    return Qnil;
}

static VALUE mAL;

void Init_al(void)
{
    mAL = rb_define_module("AL");
    
    rb_define_module_function(mAL, "Enable", al_Enable, 1);
    rb_define_module_function(mAL, "Disable", al_Disable, 1);
    rb_define_module_function(mAL, "IsEnabled", al_IsEnabled, 1);
    /*
    rb_define_module_function(mAL, "Hint", al_Hint, 2);
    */
    /*
    rb_define_module_function(mAL, "GetBooleanv", al_GetBooleanv, 1);
    rb_define_module_function(mAL, "GetIntegerv", al_GetIntegerv, 1);
    rb_define_module_function(mAL, "GetFloatv", al_GetFloatv, 1);
    rb_define_module_function(mAL, "GetDoublev", al_GetDoublev, 1);
    */
    rb_define_module_function(mAL, "GetString", al_GetString, 1);
    rb_define_module_function(mAL, "GetBoolean", al_GetBoolean, 1);
    rb_define_module_function(mAL, "GetInteger", al_GetInteger, 1);
    rb_define_module_function(mAL, "GetDouble", al_GetDouble, 1);
    rb_define_module_function(mAL, "GetError", al_GetError, 0);
    /*
    rb_define_module_function(mAL, "IsExtensionPresent", al_IsExtensionPresent, 1);
    rb_define_module_function(mAL, "GetProcAddress", al_GetProcAddress, 1);
    rb_define_module_function(mAL, "GetEnumValue", al_GetEnumValue, 1);
    */
    rb_define_module_function(mAL, "Listenerf", al_Listenerf, 2);
    rb_define_module_function(mAL, "Listeneri", al_Listeneri, 2);
    rb_define_module_function(mAL, "Listener3f", al_Listener3f, 4);
    rb_define_module_function(mAL, "Listenerfv", al_Listenerfv, 2);
    rb_define_module_function(mAL, "GetListeneri", al_GetListeneri, 1);
    rb_define_module_function(mAL, "GetListenerf", al_GetListenerf, 1);
    /*
    rb_define_module_function(mAL, "GetListeneriv", al_GetListeneriv, 1);
    rb_define_module_function(mAL, "GetListenerfv", al_GetListenerfv, 1);
    */
    rb_define_module_function(mAL, "GetListener3f", al_GetListener3f, 1);
    rb_define_module_function(mAL, "GenSources", al_GenSources, 1);
    rb_define_module_function(mAL, "DeleteSources", al_DeleteSources, 1);
    rb_define_module_function(mAL, "IsSource", al_IsSource, 1);
    rb_define_module_function(mAL, "Sourcei", al_Sourcei, 3);
    rb_define_module_function(mAL, "Sourcef", al_Sourcef, 3);
    rb_define_module_function(mAL, "Source3f", al_Source3f, 5);
    rb_define_module_function(mAL, "Sourcefv", al_Sourcefv, 3);
    rb_define_module_function(mAL, "GetSourcei", al_GetSourcei, 2);
    /*
    rb_define_module_function(mAL, "GetSourceiv", al_GetSourceiv, 2);
    */
    rb_define_module_function(mAL, "GetSourcef", al_GetSourcef, 2);
    /*
    rb_define_module_function(mAL, "GetSourcefv", al_GetSourcefv, 2);
    */
    rb_define_module_function(mAL, "SourcePlay", al_SourcePlay, 1);
    rb_define_module_function(mAL, "SourcePause", al_SourcePause, 1);
    rb_define_module_function(mAL, "SourceRewind", al_SourceRewind, 1);
    rb_define_module_function(mAL, "SourceStop", al_SourceStop, 1);
    rb_define_module_function(mAL, "GenBuffers", al_GenBuffers, 1);
    rb_define_module_function(mAL, "DeleteBuffers", al_DeleteBuffers, 1);
    rb_define_module_function(mAL, "IsBuffer", al_IsBuffer, 1);
    rb_define_module_function(mAL, "BufferData", al_BufferData, 5);
    rb_define_module_function(mAL, "GetBufferi", al_GetBufferi, 2);
    rb_define_module_function(mAL, "GetBufferf", al_GetBufferf, 2);
    /*
    rb_define_module_function(mAL, "GetBufferiv", al_GetBufferiv, 2);
    rb_define_module_function(mAL, "GetBufferfv", al_GetBufferfv, 2);
    */
    rb_define_module_function(mAL, "SourceQueueBuffers", al_SourceQueueBuffers, 2);
    rb_define_module_function(mAL, "SourceUnqueueBuffers", al_SourceUnqueueBuffers, 2);
    /*
    rb_define_module_function(mAL, "Queuei", al_Queuei, 3);
    */
    rb_define_module_function(mAL, "DopplerFactor", al_DopplerFactor, 1);
    rb_define_module_function(mAL, "DopplerVelocity", al_DopplerVelocity, 1);
    rb_define_module_function(mAL, "DistanceModel", al_DistanceModel, 1);
    
    rb_define_const(mAL, "INVALID", INT2NUM(AL_INVALID));                             
    rb_define_const(mAL, "NONE", INT2NUM(AL_NONE));                                
    rb_define_const(mAL, "FALSE", INT2NUM(AL_FALSE));                               
    rb_define_const(mAL, "TRUE", INT2NUM(AL_TRUE));                                
    rb_define_const(mAL, "SOURCE_TYPE", INT2NUM(AL_SOURCE_TYPE));                         
    rb_define_const(mAL, "SOURCE_RELATIVE", INT2NUM(AL_SOURCE_RELATIVE));                     
    rb_define_const(mAL, "CONE_INNER_ANGLE", INT2NUM(AL_CONE_INNER_ANGLE));                    
    rb_define_const(mAL, "CONE_OUTER_ANGLE", INT2NUM(AL_CONE_OUTER_ANGLE));                    
    rb_define_const(mAL, "PITCH", INT2NUM(AL_PITCH));                               
    rb_define_const(mAL, "POSITION", INT2NUM(AL_POSITION));                            
    rb_define_const(mAL, "DIRECTION", INT2NUM(AL_DIRECTION));                           
    rb_define_const(mAL, "VELOCITY", INT2NUM(AL_VELOCITY));                            
    rb_define_const(mAL, "LOOPING", INT2NUM(AL_LOOPING));                       
    rb_define_const(mAL, "BUFFER", INT2NUM(AL_BUFFER));                              
    rb_define_const(mAL, "GAIN", INT2NUM(AL_GAIN));                       
    rb_define_const(mAL, "MIN_GAIN", INT2NUM(AL_MIN_GAIN));                            
    rb_define_const(mAL, "MAX_GAIN", INT2NUM(AL_MAX_GAIN));                            
    rb_define_const(mAL, "ORIENTATION", INT2NUM(AL_ORIENTATION));                         
    rb_define_const(mAL, "SOURCE_STATE", INT2NUM(AL_SOURCE_STATE));                        
    rb_define_const(mAL, "INITIAL", INT2NUM(AL_INITIAL));                             
    rb_define_const(mAL, "PLAYING", INT2NUM(AL_PLAYING));                             
    rb_define_const(mAL, "PAUSED", INT2NUM(AL_PAUSED));                              
    rb_define_const(mAL, "STOPPED", INT2NUM(AL_STOPPED));                             
    rb_define_const(mAL, "BUFFERS_QUEUED", INT2NUM(AL_BUFFERS_QUEUED));                      
    rb_define_const(mAL, "BUFFERS_PROCESSED", INT2NUM(AL_BUFFERS_PROCESSED));                   
    rb_define_const(mAL, "PENDING", INT2NUM(AL_PENDING));                             
    rb_define_const(mAL, "PROCESSED", INT2NUM(AL_PROCESSED));                           
    rb_define_const(mAL, "FORMAT_MONO8", INT2NUM(AL_FORMAT_MONO8));                        
    rb_define_const(mAL, "FORMAT_MONO16", INT2NUM(AL_FORMAT_MONO16));                       
    rb_define_const(mAL, "FORMAT_STEREO8", INT2NUM(AL_FORMAT_STEREO8));                      
    rb_define_const(mAL, "FORMAT_STEREO16", INT2NUM(AL_FORMAT_STEREO16));                     
    rb_define_const(mAL, "REFERENCE_DISTANCE", INT2NUM(AL_REFERENCE_DISTANCE));                  
    rb_define_const(mAL, "ROLLOFF_FACTOR", INT2NUM(AL_ROLLOFF_FACTOR));                      
    rb_define_const(mAL, "CONE_OUTER_GAIN", INT2NUM(AL_CONE_OUTER_GAIN));                     
    rb_define_const(mAL, "MAX_DISTANCE", INT2NUM(AL_MAX_DISTANCE));                        
    rb_define_const(mAL, "FREQUENCY", INT2NUM(AL_FREQUENCY));                           
    rb_define_const(mAL, "BITS", INT2NUM(AL_BITS));                                
    rb_define_const(mAL, "CHANNELS", INT2NUM(AL_CHANNELS));                            
    rb_define_const(mAL, "SIZE", INT2NUM(AL_SIZE));                                
    rb_define_const(mAL, "UNUSED", INT2NUM(AL_UNUSED));                              
    rb_define_const(mAL, "NO_ERROR", INT2NUM(AL_NO_ERROR));                            
    rb_define_const(mAL, "INVALID_NAME", INT2NUM(AL_INVALID_NAME));                        
    rb_define_const(mAL, "INVALID_VALUE", INT2NUM(AL_INVALID_VALUE));                       
    rb_define_const(mAL, "OUT_OF_MEMORY", INT2NUM(AL_OUT_OF_MEMORY));                       
    rb_define_const(mAL, "VENDOR", INT2NUM(AL_VENDOR));                              
    rb_define_const(mAL, "VERSION", INT2NUM(AL_VERSION));                             
    rb_define_const(mAL, "RENDERER", INT2NUM(AL_RENDERER));                            
    rb_define_const(mAL, "EXTENSIONS", INT2NUM(AL_EXTENSIONS));                          
    rb_define_const(mAL, "DOPPLER_FACTOR", INT2NUM(AL_DOPPLER_FACTOR));                      
    rb_define_const(mAL, "DOPPLER_VELOCITY", INT2NUM(AL_DOPPLER_VELOCITY));                    
    rb_define_const(mAL, "DISTANCE_MODEL", INT2NUM(AL_DISTANCE_MODEL));                      
    rb_define_const(mAL, "INVERSE_DISTANCE", INT2NUM(AL_INVERSE_DISTANCE));                    
    rb_define_const(mAL, "INVERSE_DISTANCE_CLAMPED", INT2NUM(AL_INVERSE_DISTANCE_CLAMPED));            
    rb_define_const(mAL, "INVALID_ENUM", INT2NUM(AL_INVALID_ENUM));                        
    rb_define_const(mAL, "INVALID_OPERATION", INT2NUM(AL_INVALID_OPERATION));                   
}
