#ifndef _WIN32
#  include <dlfcn.h>
#else
#  include <windows.h>
#endif
#include "internal.h"

MIXED_EXPORT uint8_t mixed_samplesize(enum mixed_encoding encoding){
  switch(encoding){
  case MIXED_INT8: return 1;
  case MIXED_UINT8: return 1;
  case MIXED_INT16: return 2;
  case MIXED_UINT16: return 2;
  case MIXED_INT24: return 3;
  case MIXED_UINT24: return 3;
  case MIXED_INT32: return 4;
  case MIXED_UINT32: return 4;
  case MIXED_FLOAT: return 4;
  case MIXED_DOUBLE: return 8;
  default: return -1;
  }
}

int mix_noop(struct mixed_segment *segment){
  IGNORE(segment);
  return 1;
}

thread_local int errorcode = 0;

void mixed_err(int code){
  errorcode = code;
}

MIXED_EXPORT int mixed_error(){
  return errorcode;
}

MIXED_EXPORT char *mixed_error_string(int code){
  if(code < 0) code = errorcode;
  switch(code){
  case MIXED_NO_ERROR:
    return "No error has occurred.";
  case MIXED_OUT_OF_MEMORY:
    return "An allocation has failed. You are likely out of memory.";
  case MIXED_UNKNOWN_ENCODING:
    return "The specified sample encoding is unknown.";
  case MIXED_MIXING_FAILED:
    return "An error occurred during the mixing of a segment.";
  case MIXED_NOT_IMPLEMENTED:
    return "The segment function you tried to call was not provided.";
  case MIXED_NOT_INITIALIZED:
    return "An attempt was made to use an object without initializing it properly first.";
  case MIXED_INVALID_LOCATION:
    return "Cannot set the field at the specified location in the segment.";
  case MIXED_INVALID_FIELD:
    return "A field that the segment does not recognise was requested.";
  case MIXED_INVALID_VALUE:
    return "A value that is not within the valid range for a field was attempted to be set.";
  case MIXED_SEGMENT_ALREADY_STARTED:
    return "Cannot start the segment as it is already started.";
  case MIXED_SEGMENT_ALREADY_ENDED:
    return "Cannot end the segment as it is already ended.";
  case MIXED_DYNAMIC_OPEN_FAILED:
    return "Failed to open the plugin library. Most likely the file could not be read.";
  case MIXED_BAD_DYNAMIC_LIBRARY:
    return "The plugin library was found but does not seem to be a valid library as it is missing its initialisation function.";
  case MIXED_LADSPA_NO_PLUGIN_AT_INDEX:
    return "The LADSPA plugin library does not have a plugin at the requested index.";
  case MIXED_LADSPA_INSTANTIATION_FAILED:
    return "Instantiation of the LADSPA plugin has failed.";
  case MIXED_RESAMPLE_FAILED:
    return "An error happened in the libsamplerate library.";
  case MIXED_BUFFER_EMPTY:
    return "A read was requested on a buffer with no committed write data.";
  case MIXED_BUFFER_FULL:
    return "A write was requested on a buffer with no available space.";
  case MIXED_BUFFER_OVERCOMMIT:
    return "More data was attempted to be committed to the buffer than was requested.";
  case MIXED_BAD_RESAMPLE_FACTOR:
    return "The requested change in the sample rates is too big.";
  case MIXED_BAD_CHANNEL_CONFIGURATION:
    return "An unsupported channel conversion configuration was requested.";
  case MIXED_BUFFER_ALLOCATED:
    return "An allocated buffer was passed when an unallocated one was expected.";
  case MIXED_BUFFER_MISSING:
    return "An input or output port is missing a buffer.";
  case MIXED_DUPLICATE_SEGMENT:
    return "A segment with the requested name had already been registered.";
  case MIXED_BAD_SEGMENT:
    return "A segment with the requested name is not registered.";
  default:
    return "Unknown error code.";
  }
}

MIXED_EXPORT char *mixed_type_string(int code){
  switch(code){
  case MIXED_UNKNOWN:
    return "unknown";
  case MIXED_INT8:
    return "int8";
  case MIXED_UINT8:
    return "uint8";
  case MIXED_INT16:
    return "int16";
  case MIXED_UINT16:
    return "uint16";
  case MIXED_INT24:
    return "int24";
  case MIXED_UINT24:
    return "uint24";
  case MIXED_INT32:
    return "int32";
  case MIXED_UINT32:
    return "uint32";
  case MIXED_FLOAT:
    return "float";
  case MIXED_DOUBLE:
    return "double";
  case MIXED_BOOL:
    return "bool";
  case MIXED_SIZE_T:
    return "size_t";
  case MIXED_STRING:
    return "string";
  case MIXED_FUNCTION:
    return "function";
  case MIXED_POINTER:
    return "pointer";
  case MIXED_SEGMENT_POINTER:
    return "segment pointer";
  case MIXED_BUFFER_POINTER:
    return "buffer pointer";
  case MIXED_PACK_POINTER:
    return "pack pointer";
  case MIXED_SEGMENT_SEQUENCE_POINTER:
    return "segment sequence pointer";
  case MIXED_LOCATION_ENUM:
    return "location";
  case MIXED_FREQUENCY_PASS_ENUM:
    return "frequency pass";
  case MIXED_REPEAT_MODE_ENUM:
    return "repeat mode";
  case MIXED_NOISE_TYPE_ENUM:
    return "noise type";
  case MIXED_GENERATOR_TYPE_ENUM:
    return "generator type";
  case MIXED_FADE_TYPE_ENUM:
    return "fade type";
  case MIXED_ATTENUATION_ENUM:
    return "attenuation";
  case MIXED_ENCODING_ENUM:
    return "encoding";
  case MIXED_ERROR_ENUM:
    return "error";
  case MIXED_RESAMPLE_TYPE_ENUM:
    return "resample type";
  default:
    return "unknown";
  }
}

MIXED_EXPORT char *mixed_version(){
  return MIXED_VERSION;
}

void *crealloc(void *ptr, size_t oldcount, size_t newcount, size_t size){
  size_t newsize = newcount*size;
  size_t oldsize = oldcount*size;
  ptr = realloc(ptr, newsize);
  if(ptr && oldsize < newsize){
    memset(((char*)ptr)+oldsize, 0, newsize-oldsize);
  }
  return ptr;
}

void set_info_field(struct mixed_segment_field_info *info, uint32_t field, enum mixed_segment_field_type type, uint32_t count, enum mixed_segment_info_flags flags, char*description){
  info->field = field;
  info->description = description;
  info->flags = flags;
  info->type = type;
  info->type_count = count;
}

void clear_info_field(struct mixed_segment_field_info *info){
  info->field = 0;
  info->description = 0;
  info->flags = 0;
  info->type = 0;
  info->type_count = 0;
}

void *open_library(char *file){
#ifdef _WIN32
  void *lib = LoadLibrary(file);
  if(!lib){
    mixed_err(MIXED_DYNAMIC_OPEN_FAILED);
    return 0;
  }
  return lib;
#else
  void *lib = dlopen(file, RTLD_NOW);
  if(!lib){
    fprintf(stderr, "MIXED: DYLD error: %s\n", dlerror());
    mixed_err(MIXED_DYNAMIC_OPEN_FAILED);
    return 0;
  }
  dlerror();
  return lib;
#endif
}

void close_library(void *handle){
  if(handle)
#ifdef _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

void *load_symbol(void *handle, char *name){
#ifdef _WIN32
  void *function = GetProcAddress(handle, "mixed_make_plugin");
  if(!function){
    mixed_err(MIXED_BAD_DYNAMIC_LIBRARY);
    return 0;
  }
  return function;
#else
  void *function = dlsym(handle, name);
  char *error = dlerror();
  if(error != 0){
    fprintf(stderr, "MIXED: DYLD error: %s\n", error);
    mixed_err(MIXED_BAD_DYNAMIC_LIBRARY);
    return 0;
  }
  return function;
#endif
}

float mixed_random_rdrand();
float mixed_random_m(){
  return (float)rand()/(float)RAND_MAX;
}

float (*mixed_random)() = mixed_random_m;

#ifdef __RDRND__
uint8_t rdrand_available(){
  const unsigned int flag_RDRAND = (1 << 30);

  unsigned int eax, ebx, ecx, edx;
  __cpuid(1, eax, ebx, ecx, edx);

  return ((ecx & flag_RDRAND) == flag_RDRAND);
}

float mixed_random_rdrand(){
  int retries = 10;
  uint32_t random;
  while(retries--){
    if (_rdrand32_step(&random)) {
      return (float)(random/UINT32_MAX);
    }
  }
  return mixed_random_m();
}
#else
uint8_t rdrand_available(){
  return 0;
}
float mixed_random_rdrand(){
  return 0f0;
}
#endif

static void init() __attribute__((constructor));
void init(){
  srand(time(NULL));
  if(rdrand_available())
    mixed_random = mixed_random_rdrand;
}
