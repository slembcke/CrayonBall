#if defined(__APPLE_CC__)
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
#endif

#include <ruby.h>
#include <intern.h>
#include "rboal.h"

static VALUE cALCcontext;
static VALUE cALCdevice;

static VALUE alc_CreateContext(VALUE obj, VALUE arg_dev, VALUE arg_attrlist)
{
    ALCdevice *dev;
    Data_Get_Struct(arg_dev, ALCdevice, dev);
    unsigned int attrlist_count = rb_ary_size(arg_attrlist);
    ALint *attrlist = alloca(sizeof(ALint) * attrlist_count);
    unsigned int attrlist_index;
    for (attrlist_index = 0; attrlist_index < attrlist_count; ++attrlist_index)
    {
        attrlist[attrlist_index] = (ALint)NUM2INT(rb_ary_entry(arg_attrlist, attrlist_index));
    }
    ALCcontext *result = alcCreateContext(dev, attrlist);
    return Data_Wrap_Struct(cALCcontext, NULL, NULL, result);
}

static VALUE alc_MakeContextCurrent(VALUE obj, VALUE arg_alcHandle)
{
    ALCcontext *alcHandle;
    if(arg_alcHandle!=Qnil){
        Data_Get_Struct(arg_alcHandle, ALCcontext, alcHandle);
    } else {
        alcHandle = NULL;
    }
    ALCenum result = alcMakeContextCurrent(alcHandle);
    return INT2FIX(result);
}

static VALUE alc_ProcessContext(VALUE obj, VALUE arg_alcHandle)
{
    ALCcontext *alcHandle;
    Data_Get_Struct(arg_alcHandle, ALCcontext, alcHandle);
    alcProcessContext(alcHandle);
    return Qnil;    
}

static VALUE alc_SuspendContext(VALUE obj, VALUE arg_alcHandle)
{
    ALCcontext *alcHandle;
    Data_Get_Struct(arg_alcHandle, ALCcontext, alcHandle);
    alcSuspendContext(alcHandle);
    return Qnil;    
}

static VALUE alc_DestroyContext(VALUE obj, VALUE arg_alcHandle)
{
    ALCcontext *alcHandle;
    Data_Get_Struct(arg_alcHandle, ALCcontext, alcHandle);
    alcDestroyContext(alcHandle);
    return Qnil;
}

static VALUE alc_GetError(VALUE obj, VALUE arg_dev)
{
    ALCdevice *dev;
    Data_Get_Struct(arg_dev, ALCdevice, dev);
    ALCenum result = alcGetError(dev);
    return INT2FIX(result);
}

static VALUE alc_GetCurrentContext(VALUE obj)
{
    ALCcontext *result = alcGetCurrentContext();
    return Data_Wrap_Struct(cALCcontext, NULL, NULL, result);
}

static VALUE alc_OpenDevice(VALUE obj, VALUE arg_tokstr)
{
    ALchar *tokstr = (ALchar *)STR2CSTR(arg_tokstr);
    ALCdevice *result = alcOpenDevice(tokstr);
    return Data_Wrap_Struct(cALCdevice, NULL, NULL, result);
}

static VALUE alc_CloseDevice(VALUE obj, VALUE arg_dev)
{
    ALCdevice *dev;
    Data_Get_Struct(arg_dev, ALCdevice, dev);
    alcCloseDevice(dev);
    return Qnil;
}

/*
static VALUE alc_IsExtensionPresent(VALUE obj, VALUE arg_device, VALUE arg_extName);
static VALUE alc_GetProcAddress(VALUE obj, VALUE arg_device, VALUE arg_funcName);
static VALUE alc_GetEnumValue(VALUE obj, VALUE arg_device, VALUE arg_enumName);
*/

static VALUE alc_GetContextsDevice(VALUE obj, VALUE arg_context)
{
    ALCcontext *context;
    Data_Get_Struct(arg_context, ALCcontext, context);
    ALCdevice *result = alcGetContextsDevice(context);
    return Data_Wrap_Struct(cALCdevice, NULL, NULL, result);
}

static VALUE alc_GetString(VALUE obj, VALUE arg_deviceHandle, VALUE arg_token)
{
    ALCdevice *deviceHandle;
    Data_Get_Struct(arg_deviceHandle, ALCdevice, deviceHandle);
    ALenum token = (ALenum)NUM2INT(arg_token);
    const ALchar *result = alcGetString(deviceHandle, token);
    return rb_str_new2((const char *)result);
}

static VALUE alc_GetIntegerv(VALUE obj, VALUE arg_deviceHandle, VALUE arg_token, VALUE arg_size)
{
    ALCdevice *deviceHandle;
    Data_Get_Struct(arg_deviceHandle, ALCdevice, deviceHandle);
    ALenum token = (ALenum)NUM2INT(arg_token);
    ALsizei size = (ALsizei)NUM2INT(arg_size);
    ALint *dest = alloca(sizeof(ALint) * size);
    alcGetIntegerv(deviceHandle, token, size, dest);
    VALUE result = rb_ary_new();
    ALsizei dest_index;
    for (dest_index = 0; dest_index < size; ++dest_index)
    {
        rb_ary_push(result, INT2NUM(dest[dest_index]));
    }
    return result;
}

static VALUE mALC;

void Init_alc()
{
    mALC = rb_define_module("ALC");
    
    cALCcontext = rb_define_class_under(mALC, "ALCcontext", rb_cObject);
    cALCdevice = rb_define_class_under(mALC, "ALCdevice", rb_cObject);
    
    rb_define_module_function(mALC, "CreateContext", alc_CreateContext, 2);
    rb_define_module_function(mALC, "MakeContextCurrent", alc_MakeContextCurrent, 1);
    rb_define_module_function(mALC, "ProcessContext", alc_ProcessContext, 1);
    rb_define_module_function(mALC, "SuspendContext", alc_SuspendContext, 1);
    rb_define_module_function(mALC, "DestroyContext", alc_DestroyContext, 1);
    rb_define_module_function(mALC, "GetError", alc_GetError, 1);
    rb_define_module_function(mALC, "GetCurrentContext", alc_GetCurrentContext, 0);
    rb_define_module_function(mALC, "OpenDevice", alc_OpenDevice, 1);
    rb_define_module_function(mALC, "CloseDevice", alc_CloseDevice, 1);
    /*
    rb_define_module_function(mALC, "IsExtensionPresent", alc_IsExtensionPresent, xxxxx);
    rb_define_module_function(mALC, "GetProcAddress", alc_GetProcAddress, xxxxx);
    rb_define_module_function(mALC, "GetEnumValue", alc_GetEnumValue, xxxxx);
    */
    rb_define_module_function(mALC, "GetContextsDevice", alc_GetContextsDevice, 1);
    rb_define_module_function(mALC, "GetString", alc_GetString, 2);
    rb_define_module_function(mALC, "GetIntegerv", alc_GetIntegerv, 3);
    
    rb_define_const(mALC, "INVALID", INT2NUM(ALC_INVALID));                 
    rb_define_const(mALC, "FREQUENCY", INT2NUM(ALC_FREQUENCY));               
    rb_define_const(mALC, "REFRESH", INT2NUM(ALC_REFRESH));                 
    rb_define_const(mALC, "SYNC", INT2NUM(ALC_SYNC));                    
    rb_define_const(mALC, "NO_ERROR", INT2NUM(ALC_NO_ERROR));                
    rb_define_const(mALC, "INVALID_DEVICE", INT2NUM(ALC_INVALID_DEVICE));          
    rb_define_const(mALC, "INVALID_CONTEXT", INT2NUM(ALC_INVALID_CONTEXT));         
    rb_define_const(mALC, "INVALID_ENUM", INT2NUM(ALC_INVALID_ENUM));            
    rb_define_const(mALC, "INVALID_VALUE", INT2NUM(ALC_INVALID_VALUE));           
    rb_define_const(mALC, "OUT_OF_MEMORY", INT2NUM(ALC_OUT_OF_MEMORY));           
    rb_define_const(mALC, "DEFAULT_DEVICE_SPECIFIER", INT2NUM(ALC_DEFAULT_DEVICE_SPECIFIER));
    rb_define_const(mALC, "DEVICE_SPECIFIER", INT2NUM(ALC_DEVICE_SPECIFIER));        
    rb_define_const(mALC, "EXTENSIONS", INT2NUM(ALC_EXTENSIONS));              
    rb_define_const(mALC, "MAJOR_VERSION", INT2NUM(ALC_MAJOR_VERSION));           
    rb_define_const(mALC, "MINOR_VERSION", INT2NUM(ALC_MINOR_VERSION));           
    rb_define_const(mALC, "ATTRIBUTES_SIZE", INT2NUM(ALC_ATTRIBUTES_SIZE));         
    rb_define_const(mALC, "ALL_ATTRIBUTES", INT2NUM(ALC_ALL_ATTRIBUTES));          
    rb_define_const(mALC, "FALSE", INT2NUM(ALC_FALSE));                   
    rb_define_const(mALC, "TRUE", INT2NUM(ALC_TRUE));                    
}
