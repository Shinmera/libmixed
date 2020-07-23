#define __TEST_SUITE packer
#include "tester.h"
#include <math.h>

static int make_pack(enum mixed_encoding encoding, int channels, struct mixed_pack *pack){
  int samples = channels*500;
  pack->size = samples*mixed_samplesize(encoding);
  pack->encoding = encoding;
  pack->channels = channels;
  pack->samplerate = 1;
  if(!mixed_make_pack(16*mixed_samplesize(encoding), pack))
    return 0;
  char *buffer;
  size_t size = SIZE_MAX;
  mixed_pack_request_write(&buffer, &size, pack);
  for(int i=0; i<size; ++i)
    buffer[i] = rand()%128;
  mixed_pack_finish_write(size, pack);
  return 1;
}

define_test(single_channel_no_resample, {
    struct mixed_pack pack_i = {0};
    struct mixed_pack pack_o = {0};
    struct mixed_buffer buffer = {0};
    struct mixed_segment packer = {0};
    struct mixed_segment unpacker = {0};
    // Allocate stuff
    pass(make_pack(MIXED_INT8, 1, &pack_i));
    pass(make_pack(MIXED_INT8, 1, &pack_o));
    pass(mixed_make_buffer(pack_i.frames, &buffer));
    pass(mixed_make_segment_unpacker(&pack_i, pack_i.samplerate, &unpacker));
    pass(mixed_make_segment_packer(&pack_o, pack_o.samplerate, &packer));
    // Connect the buffers
    pass(mixed_segment_set_out(MIXED_BUFFER, MIXED_MONO, &buffer, &unpacker));
    pass(mixed_segment_set_in(MIXED_BUFFER, MIXED_MONO, &buffer, &packer));
    // Fill data
    int8_t *data_i = (int8_t *)pack_i.data;
    int8_t *data_o = (int8_t *)pack_o.data;
    for(int i=0; i<pack_i.size; ++i)
      data_i[i] = i % 128;
    // Run
    pass(mixed_segment_mix(&unpacker));
    pass(mixed_segment_mix(&packer));
    // Check
    is(pack_o.frames, pack_i.frames);
    for(int i=0; i<pack_i.size; ++i)
      is(data_o[i], data_i[i]);

  cleanup:
    mixed_free_segment(&packer);
    mixed_free_segment(&unpacker);
    mixed_free_buffer(&buffer);
    mixed_free_pack(&pack_i);
    mixed_free_pack(&pack_o);
  })

define_test(single_channel_downsample_out, {
    struct mixed_pack pack_i = {0};
    struct mixed_pack pack_o = {0};
    struct mixed_buffer buffer = {0};
    struct mixed_segment unpacker = {0};
    struct mixed_segment packer = {0};
    // Allocate stuff
    pass(make_pack(MIXED_INT8, 1, &pack_i));
    pass(make_pack(MIXED_INT8, 1, &pack_o));
    pack_o.samplerate = pack_i.samplerate / 2;
    pass(mixed_make_buffer(pack_i.frames, &buffer));
    pass(mixed_make_segment_unpacker(&pack_i, pack_i.samplerate, &unpacker));
    pass(mixed_make_segment_packer(&pack_o, pack_i.samplerate, &packer));
    // Set quality so we get predictable sample counts
    enum mixed_resample_type quality = MIXED_LINEAR_INTERPOLATION;
    pass(mixed_segment_set(MIXED_RESAMPLE_TYPE, &quality, &unpacker));
    pass(mixed_segment_set(MIXED_RESAMPLE_TYPE, &quality, &packer));
    // Connect the buffers
    pass(mixed_segment_set_out(MIXED_BUFFER, MIXED_MONO, &buffer, &unpacker));
    pass(mixed_segment_set_in(MIXED_BUFFER, MIXED_MONO, &buffer, &packer));
    // Fill data
    int8_t *data_i = (int8_t *)pack_i.data;
    int8_t *data_o = (int8_t *)pack_o.data;
    for(int i=0; i<pack_i.size; ++i)
      data_i[i] = rand()%128;
    // Run
    pass(mixed_segment_mix(&unpacker));
    pass(mixed_segment_mix(&packer));
    // Check for expected
    is(pack_o.frames, pack_i.frames / 2);

  cleanup:
    mixed_free_segment(&packer);
    mixed_free_segment(&unpacker);
    mixed_free_buffer(&buffer);
    mixed_free_pack(&pack_i);
    mixed_free_pack(&pack_o);
  })

define_test(single_channel_resample_in_out, {
    struct mixed_pack pack_i = {0};
    struct mixed_pack pack_o = {0};
    struct mixed_buffer buffer = {0};
    struct mixed_segment unpacker = {0};
    struct mixed_segment packer = {0};
    // Allocate stuff
    pass(make_pack(MIXED_FLOAT, 1, &pack_i));
    pass(make_pack(MIXED_FLOAT, 1, &pack_o));
    pass(mixed_make_buffer(pack_i.frames, &buffer));
    pass(mixed_make_segment_unpacker(&pack_i, 80, &unpacker));
    pass(mixed_make_segment_packer(&pack_o, 80, &packer));
    // Set quality so we get predictable results
    enum mixed_resample_type quality = MIXED_SINC_BEST_QUALITY;
    pass(mixed_segment_set(MIXED_RESAMPLE_TYPE, &quality, &unpacker));
    pass(mixed_segment_set(MIXED_RESAMPLE_TYPE, &quality, &packer));
    // Connect the buffers
    pass(mixed_segment_set_out(MIXED_BUFFER, MIXED_MONO, &buffer, &unpacker));
    pass(mixed_segment_set_in(MIXED_BUFFER, MIXED_MONO, &buffer, &packer));
    // Fill data
    float *data_i = (float *)pack_i.data;
    float *data_o = (float *)pack_o.data;
    for(int i=0; i<pack_i.frames; ++i)
      data_i[i] = ((float)i) / pack_i.size;
    // Run
    pass(mixed_segment_mix(&unpacker));
    pass(mixed_segment_mix(&packer));
    // Check for expected
    for(int i=0; i<pack_o.frames; ++i)
      is(fabs(data_o[i]-data_i[i]) < 0.00005, 1);

  cleanup:
    mixed_free_segment(&packer);
    mixed_free_segment(&unpacker);
    mixed_free_buffer(&buffer);
    mixed_free_pack(&pack_i);
    mixed_free_pack(&pack_o);
  })
  
#undef __TEST_SUITE
