#include "../internal.h"

struct basic_mixer_data{
  struct mixed_buffer **in;
  uint32_t count;
  uint32_t size;
  struct mixed_buffer **out;
  channel_t channels;
  float volume;
};

int basic_mixer_free(struct mixed_segment *segment){
  free_vector((struct vector *)segment->data);
  return 1;
}

int basic_mixer_start(struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;
  if(data->count % data->channels != 0){
    mixed_err(MIXED_BUFFER_MISSING);
    return 0;
  }
  for(channel_t i=0; i<data->channels; ++i){
    if(data->out[i] == 0){
      mixed_err(MIXED_BUFFER_MISSING);
      return 0;
    }
  }
  return 1;
}

int basic_mixer_set_out(uint32_t field, uint32_t location, void *buffer, struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;
  
  switch(field){
  case MIXED_BUFFER:
    if(data->channels <= location){
      mixed_err(MIXED_INVALID_LOCATION);
      return 0;
    }
    data->out[location] = (struct mixed_buffer *)buffer;
    return 1;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
}

int basic_mixer_set_in(uint32_t field, uint32_t location, void *buffer, struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;

  switch(field){
  case MIXED_BUFFER:
    if(buffer){ // Add or set an element
      if(location < data->count){
        data->in[location] = (struct mixed_buffer *)buffer;
      }else{
        return vector_add(buffer, (struct vector *)data);
      }
    }else{ // Remove an element
      if(data->count <= location){
        mixed_err(MIXED_INVALID_LOCATION);
        return 0;
      }
      return vector_remove_pos(location, (struct vector *)data);
    }
    return 1;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
}

VECTORIZE int basic_mixer_mix(struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;
  channel_t channels = data->channels;
  uint32_t buffers = data->count / channels;
  if(buffers == 0){
    for(channel_t c=0; c<channels; ++c){
      float *out;
      uint32_t samples = UINT32_MAX;
      mixed_buffer_request_write(&out, &samples, data->out[c]);
      memset(out, 0, samples*sizeof(float));
      mixed_buffer_finish_write(samples, data->out[c]);
    }
  }else{
    float volume = data->volume;
    float div = volume;

    for(channel_t c=0; c<channels; ++c){
      float *in=0, *out=0;
      
      // Compute how much we can mix on this channel.
      uint32_t samples = mixed_buffer_available_write(data->out[c]);
      for(uint32_t i=c; i<data->count; i+=channels)
        mixed_buffer_request_read(&in, &samples, data->in[i]);

      mixed_buffer_request_write(&out, &samples, data->out[c]);

      // Mix first buffer directly.
      struct mixed_buffer *buffer = data->in[c];
      mixed_buffer_request_read(&in, &samples, buffer);
      for(uint32_t i=0; i<samples; ++i){
        out[i] = in[i]*div;
      }
      mixed_buffer_finish_read(samples, buffer);

      // Mix other buffers additively.
      for(uint32_t i=c+channels; i<data->count; i+=channels){
        struct mixed_buffer *buffer = data->in[i];
        mixed_buffer_request_read(&in, &samples, buffer);
        for(uint32_t j=0; j<samples; ++j){
          out[j] += in[j]*div;
        }
        mixed_buffer_finish_read(samples, buffer);
      }
      mixed_buffer_finish_write(samples, data->out[c]);
    }
  }
  return 1;
}

// FIXME: add start method that checks for buffer completeness.
int basic_mixer_set(uint32_t field, void *value, struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;
  
  switch(field){
  case MIXED_VOLUME:
    data->volume = *((float *)value);
    return 1;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
}

int basic_mixer_get(uint32_t field, void *value, struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;
  
  switch(field){
  case MIXED_VOLUME:
    *((float *)value) = data->volume;
    return 1;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
}

int basic_mixer_info(struct mixed_segment_info *info, struct mixed_segment *segment){
  struct basic_mixer_data *data = (struct basic_mixer_data *)segment->data;
  info->name = "basic_mixer";
  info->description = "Mixes multiple buffers together";
  info->min_inputs = 0;
  info->max_inputs = -1;
  info->outputs = data->channels;
  
  struct mixed_segment_field_info *field = info->fields;
  set_info_field(field++, MIXED_BUFFER,
                 MIXED_BUFFER_POINTER, 1, MIXED_IN | MIXED_OUT | MIXED_SET,
                 "The buffer for audio data attached to the location.");

  set_info_field(field++, MIXED_VOLUME,
                 MIXED_FLOAT, 1, MIXED_SEGMENT | MIXED_SET | MIXED_GET,
                 "The volume scaling factor for the output.");
  clear_info_field(field++);
  return 1;
}

MIXED_EXPORT int mixed_make_segment_basic_mixer(channel_t channels, struct mixed_segment *segment){
  struct basic_mixer_data *data = calloc(1, sizeof(struct basic_mixer_data));
  if(!data){
    mixed_err(MIXED_OUT_OF_MEMORY);
    return 0;
  }

  data->volume = 1.0f;
  data->channels = channels;
  data->out = calloc(channels, sizeof(struct mixed_buffer *));
  if(!data->out){
    mixed_err(MIXED_OUT_OF_MEMORY);
    free(data);
    return 0;
  }
  
  segment->free = basic_mixer_free;
  segment->start = basic_mixer_start;
  segment->mix = basic_mixer_mix;
  segment->set = basic_mixer_set;
  segment->get = basic_mixer_get;
  segment->set_in = basic_mixer_set_in;
  segment->set_out = basic_mixer_set_out;
  segment->info = basic_mixer_info;
  segment->data = data;
  return 1;
}

int __make_basic_mixer(void *args, struct mixed_segment *segment){
  return mixed_make_segment_basic_mixer(ARG(channel_t, 0), segment);
}

REGISTER_SEGMENT(basic_mixer, __make_basic_mixer, 1, {{.description = "channels", .type = MIXED_UINT8}})
