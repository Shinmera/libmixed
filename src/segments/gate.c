#include "internal.h"

enum state{
  CLOSED,
  ATTACKING,
  OPEN,
  HOLDING,
  RELEASING,
};

struct gate_segment_data{
  struct mixed_buffer *in;
  struct mixed_buffer *out;
  float close_threshold;
  float open_threshold;
  float attack;
  float hold;
  float release;
  size_t samplerate;
  float time;
  enum state state;
};

float db_to_linear(float db){
  return pow(10, db/20.0);
}

float linear_to_db(float linear){
  return log10(linear)*20;
}

int gate_segment_free(struct mixed_segment *segment){
  if(segment->data){
    free(segment->data);
  }
  segment->data = 0;
  return 1;
}

// FIXME: add start method that checks for buffer completeness.

int gate_segment_start(struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;
  data->time = 0.0;
  return 1;
}

int gate_segment_set_in(size_t field, size_t location, void *buffer, struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;

  switch(field){
  case MIXED_BUFFER:
    if(location == 0){
      data->in = (struct mixed_buffer *)buffer;
      return 1;
    }
    mixed_err(MIXED_INVALID_LOCATION);
    return 0;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
}

int gate_segment_set_out(size_t field, size_t location, void *buffer, struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;

  switch(field){
  case MIXED_BUFFER:
    if(location == 0){
      data->out = (struct mixed_buffer *)buffer;
      return 1;
    }
    mixed_err(MIXED_INVALID_LOCATION);
    return 0;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
}

void gate_segment_mix(size_t samples, struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;

  float stime = 1.0/data->samplerate;
  float time = data->time;
  float open = data->open_threshold;
  float close = data->close_threshold;
  float attack = data->attack;
  float hold = data->hold;
  float release = data->release;
  float *in = data->in->data;
  float *out = data->out->data;
  float volume = 0.0;
  for(size_t i=0; i<samples; ++i){
    float sample = in[i];
    switch(data->state){
    case CLOSED:
      if(sample < open){
        volume = 0.0;
        break;
      }else{
        time = 0.0;
        data->state = ATTACKING;
      }
    case ATTACKING:
      if(sample < close){
        data->state = RELEASING;
      }else if(time < attack){
        time += stime;
        volume = time/attack;
        break;
      }else{
        data->state = OPEN;
      }
    case OPEN:
      if(close < sample){
        volume = 1.0;
        break;
      }else{
        time = hold;
        data->state = HOLDING; 
      }
    case HOLDING:
      if(open <= sample){
        data->state = OPEN;
        volume = 1.0;
      }else if(0 < time){
        time -= stime;
        volume = 1.0;
        break;
      }else{
        time = release;
        data->state = RELEASING;
      }
    case RELEASING:
      if(open <= sample){
        data->state = ATTACKING;
        volume = time/release;
      }else if(0 < time){
        time -= stime;
        volume = time/release;
      }else{
        time = 0.0;
        volume = 0.0;
        data->state = CLOSED;
      }
    }
    out[i] = sample * volume;
  }
  data->time = time;
}

void gate_segment_mix_bypass(size_t samples, struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;
  
  mixed_buffer_copy(data->in, data->out);
}

struct mixed_segment_info *gate_segment_info(struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;
  struct mixed_segment_info *info = calloc(1, sizeof(struct mixed_segment_info));

  if(info){
    info->name = "gate";
    info->description = "A noise gate segment to filter out low-volume frequencies.";
    info->flags = MIXED_INPLACE;
    info->min_inputs = 1;
    info->max_inputs = 1;
    info->outputs = 1;
  
    info->fields[0].field = MIXED_BUFFER;
    info->fields[0].description = "The buffer for audio data attached to the location.";
    info->fields[0].flags = MIXED_IN | MIXED_OUT | MIXED_SET;

    info->fields[1].field = MIXED_GATE_OPEN_THRESHOLD;
    info->fields[1].description = "The volume in dB necessary to open the gate.";
    info->fields[1].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;

    info->fields[1].field = MIXED_GATE_CLOSE_THRESHOLD;
    info->fields[1].description = "The volume in dB below which the gate is closed again.";
    info->fields[1].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;

    info->fields[1].field = MIXED_GATE_ATTACK;
    info->fields[1].description = "The time during which the output volume is scaled up.";
    info->fields[1].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;

    info->fields[1].field = MIXED_GATE_HOLD;
    info->fields[1].description = "The time during which the output is still transmitted despite the gate being closed.";
    info->fields[1].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;

    info->fields[1].field = MIXED_GATE_RELEASE;
    info->fields[1].description = "The time during which the output volume is scaled down.";
    info->fields[1].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;

    info->fields[2].field = MIXED_SAMPLERATE;
    info->fields[2].description = "The samplerate at which the segment operates.";
    info->fields[2].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;

    info->fields[5].field = MIXED_BYPASS;
    info->fields[5].description = "Bypass the segment's processing.";
    info->fields[5].flags = MIXED_SEGMENT | MIXED_SET | MIXED_GET;
  }

  return info;
}

int gate_segment_get(size_t field, void *value, struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;
  switch(field){
  case MIXED_GATE_OPEN_THRESHOLD: *((float *)value) = linear_to_db(data->open_threshold); break;
  case MIXED_GATE_CLOSE_THRESHOLD: *((float *)value) = linear_to_db(data->close_threshold); break;
  case MIXED_GATE_ATTACK: *((float *)value) = data->attack; break;
  case MIXED_GATE_HOLD: *((float *)value) = data->hold; break;
  case MIXED_GATE_RELEASE: *((float *)value) = data->release; break;
  case MIXED_SAMPLERATE: *((size_t *)value) = data->samplerate; break;
  case MIXED_BYPASS: *((bool *)value) = (segment->mix == gate_segment_mix_bypass); break;
  default: mixed_err(MIXED_INVALID_FIELD); return 0;
  }
  return 1;
}

int gate_segment_set(size_t field, void *value, struct mixed_segment *segment){
  struct gate_segment_data *data = (struct gate_segment_data *)segment->data;
  switch(field){
  case MIXED_SAMPLERATE:
    if(*(size_t *)value <= 0){
      mixed_err(MIXED_INVALID_VALUE);
      return 0;
    }
    break;
  case MIXED_GATE_OPEN_THRESHOLD:
    data->open_threshold = db_to_linear(*(float *)value);
    break;
  case MIXED_GATE_CLOSE_THRESHOLD:
    data->close_threshold = db_to_linear(*(float *)value);
    break;
  case MIXED_GATE_ATTACK:
    if(*(float *)value < 0.0){
      mixed_err(MIXED_INVALID_VALUE);
      return 0;
    }
    data->attack = *(float *)value;
    break;
  case MIXED_GATE_HOLD:
    if(*(float *)value < 0.0){
      mixed_err(MIXED_INVALID_VALUE);
      return 0;
    }
    data->hold = *(float *)value;
    break;
  case MIXED_GATE_RELEASE:
    if(*(float *)value < 0.0){
      mixed_err(MIXED_INVALID_VALUE);
      return 0;
    }
    data->release = *(float *)value;
    break;
  case MIXED_BYPASS:
    if(*(bool *)value){
      segment->mix = gate_segment_mix_bypass;
    }else{
      segment->mix = gate_segment_mix;
    }
    break;
  default:
    mixed_err(MIXED_INVALID_FIELD);
    return 0;
  }
  return 1;
}

MIXED_EXPORT int mixed_make_segment_gate(size_t samplerate, struct mixed_segment *segment){
  struct gate_segment_data *data = calloc(1, sizeof(struct gate_segment_data));
  if(!data){
    mixed_err(MIXED_OUT_OF_MEMORY);
    return 0;
  }

  data->samplerate = samplerate;
  data->open_threshold = db_to_linear(-24.0);
  data->close_threshold = db_to_linear(-32.0);
  data->attack = 0.025;
  data->hold = 0.2;
  data->release = 0.15;
  
  segment->free = gate_segment_free;
  segment->start = gate_segment_start;
  segment->mix = gate_segment_mix;
  segment->set_in = gate_segment_set_in;
  segment->set_out = gate_segment_set_out;
  segment->info = gate_segment_info;
  segment->get = gate_segment_get;
  segment->set = gate_segment_set;
  segment->data = data;
  return 1;
}
