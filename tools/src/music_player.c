#include <stdio.h>

#define FS_IMPLEMENTATION
#include <core/fs.h>

#define WAV_IMPLEMENTATION
#include <core/wav.h>

#define AUDIO_IMPLEMENTATION
#include <core/audio.h>

#include <core/types.h>

#define AAC_IMPLEMENTATION
#include <core/aac.h>

#define MP4_IMPLEMENTATION
#include <core/mp4.h>

#define MINIMP3_IMPLEMENTATION
#include <thirdparty/minimp3.h>

#define DR_FLAC_IMPLEMENTATION
#include <thirdparty/dr_flac.h>

#include <thirdparty/stb_vorbis.h>

#define PL_MPEG_IMPLEMENTATION
#include <thirdparty/pl_mpeg.h>

typedef enum {
  FORMAT_ERROR,
  FORMAT_NONE,

  FORMAT_WAV,
  FORMAT_MP3,
  FORMAT_FLAC,
  FORMAT_VORBIS,
  FORMAT_MPG,
  FORMAT_AAC,
  FORMAT_MP4_AAC,
} Format;

typedef struct {
  u8 *data;
  u64 len;
  u64 pos;
} Memory;

int mem_read(void *opaque, u8 *data, u64 len) {

  Memory *m = opaque;

  if(m->pos + len > m->len) {
    return 0;
  }

  memcpy(data, m->data + m->pos, len);
  m->pos += len;

  return 1;
}

int mem_seek(void *opaque, u64 offset) {
  Memory *m = opaque;

  if(offset > m->len) {
    return 0;
  }
  m->pos = offset;

  return 1;
}

size_t flac_mem_read(void *opaque, void *buf, size_t to_read) {
  Memory *m = opaque;

  if(m->pos + to_read > m->len) {
    return 0;
  }

  memcpy(buf, m->data + m->pos, to_read);
  m->pos += to_read;

  return to_read;
}

drflac_bool32 flac_mem_seek(void *opaque,
			    int offset,
			    drflac_seek_origin origin) {
  return 0;
}

typedef struct {
  mp3dec_t dec;
  mp3dec_frame_info_t info;
} Mp3;

typedef struct {
  Mp4_Decoder dec;
  Aac aac;
} Mp4;

typedef union {
  Wav_Decoder wav;
  Mp3 mp3;
  drflac *flac;
  stb_vorbis *vorbis;
  plm_t *mpg;
  Aac aac;
  Mp4 mp4;
} Context;

Format probe(Context *c, Memory *m, int *flt_or_pcm, s32 *channels, s32 *sample_rate) {

  if(wav_decoder_open(&c->wav,
		      m,
		      mem_read,
		      mem_seek)) {
    if(c->wav.format == WAV_FORMAT_IEEE) {
      *flt_or_pcm = 0;
    } else if(c->wav.format == WAV_FORMAT_PCM) {
      *flt_or_pcm = 1;
    } else {
      return FORMAT_ERROR;
    }
    *channels = c->wav.channels;
    *sample_rate = c->wav.sample_rate;
    return FORMAT_WAV;
  }
  m->pos = 0;

  mp3dec_init(&c->mp3.dec);
  if(mp3dec_decode_frame(&c->mp3.dec,
			 m->data,
			 m->len,
			 NULL,
			 &c->mp3.info) > 0) {
    *flt_or_pcm = 1;
    *channels = c->mp3.info.channels;
    *sample_rate = c->mp3.info.hz;
    return FORMAT_MP3;
  }
  m->pos = 0;

  c->flac = drflac_open(flac_mem_read,
			flac_mem_seek,
			m,
			NULL);
  m->pos = 0;
  if(c->flac != NULL) {
    *flt_or_pcm = 1;
    *channels = c->flac->channels;
    *sample_rate = c->flac->sampleRate;
    return FORMAT_FLAC;
  }

  s32 consumed, error;
  c->vorbis = stb_vorbis_open_pushdata(m->data,
				       m->len,
				       &consumed,
				       &error,
				       NULL);
  m->pos = 0;
  if(c->vorbis != NULL) {
    *flt_or_pcm = 0;
    *channels = c->vorbis->channels;
    *sample_rate = c->vorbis->sample_rate;
    return FORMAT_VORBIS;
  }

  c->mpg = plm_create_with_memory(m->data,
				  m->len,
				  0);
  m->pos = 0;
  if(c->mpg != NULL && plm_probe(c->mpg, m->len)) {
    *flt_or_pcm = 0;
    *channels = 2;
    *sample_rate = plm_get_samplerate(c->mpg);
    return FORMAT_MPG;
  }

  memset(&c->aac, 0, sizeof(c->aac));
  if(aac_decode(&c->aac, m->data, m->len, NULL) == AAC_ERROR_NONE) {
    *flt_or_pcm = 1;
    *channels = c->aac.channels;
    *sample_rate = c->aac.sample_rate;
    return FORMAT_AAC;
  }
  m->pos = 0;

  if(mp4_decoder_open(&c->mp4.dec,
		      m,
		      mem_read,
		      mem_seek)) {
    if(c->mp4.dec.type == MP4_MEDIA_TYPE_AUDIO &&
       c->mp4.dec.codec == MP4_CODEC_mp4a) {
      *flt_or_pcm = 1;
      *channels = c->mp4.dec.as.audio.channels;
      *sample_rate = c->mp4.dec.as.audio.sample_rate;
      return FORMAT_MP4_AAC;
    }
  }
  m->pos = 0;

  return FORMAT_NONE;
}

int main(int argc, char **argv) {

  if(argc < 2) {
    fprintf(stderr, "ERROR: Please provide an argument\n");
    fprintf(stderr, "USAGE: %s <audio-file>\n", argv[0]);
    fflush(stderr);
    return 1;
  }
  char *filepath = argv[1];

  Memory mem;
  if(fs_slurp_filec(filepath, &mem.data, &mem.len) != FS_ERROR_NONE) {
    fprintf(stderr, "ERROR: Cannot slurp file: '%s'\n", filepath); return 1;
  }

  int flt_or_pcm;
  s32 channels, sample_rate;
  Context context;
  Format format = probe(&context, &mem, &flt_or_pcm, &channels, &sample_rate);
  if(format == FORMAT_NONE) {
    fprintf(stderr, "ERROR: Cannot decode file: '%s'\n", filepath);
    return 1;
  } else if(format ==  FORMAT_ERROR) {
    fprintf(stderr, "ERROR: happened during decoding\n");
    return 1;
  }

  Audio_Fmt fmt;
  if(flt_or_pcm == 0) {
    fmt = AUDIO_FMT_FLT;
  } else {
    fmt = AUDIO_FMT_S16;
  }

  printf("flt_or_pcm: %d\n", flt_or_pcm);
  printf("channels: %d\n", channels);
  printf("sample_rate: %d\n", sample_rate);

  Audio audio;
  if(!audio_init(&audio, fmt, channels, sample_rate)) {
    fprintf(stderr, "ERROR: Cannot initialize audio\n");
    return 1;
  }

  u8 cache[1024];
  u64 cache_len = 0;

#define MAX_SAMPLES 1152
#define MAX_CHANNELS 2
#define MAX_SIZEOF_SAMPLE 4 * MAX_CHANNELS

  u8 buf[2][MAX_SAMPLES * 2 * MAX_SIZEOF_SAMPLE];
  s32 current = 0;

  switch(format) {
  case FORMAT_WAV: {
    u32 samples;
    while(wav_decoder_decode(&context.wav, buf[current], sizeof(buf[current]), &samples)) {
      audio_play(&audio, buf[current], samples);

      current = 1 - current;
    }
  }  break;
  case FORMAT_MP3: {
    while(mem.pos != mem.len) {
      s32 samples = mp3dec_decode_frame(&context.mp3.dec,
					mem.data + mem.pos,
					mem.len - mem.pos,
					(s16 *) buf[current],
					&context.mp3.info);

      if(samples > 0) {
	if(context.mp3.info.frame_bytes > 0) {
	  audio_play(&audio, buf[current], samples);
	  current = 1 - current;

	} else {
	  UNREACHABLE();
	}
      } else {
	if(context.mp3.info.frame_bytes > 0) {
	  // ignore
	} else {
	  // insufficient data
	  break;
	}
      }

      mem.pos += context.mp3.info.frame_bytes;
    }
  }break;
  case FORMAT_FLAC: {
    while(1) {
      u64 frames_read = drflac_read_pcm_frames_s16(context.flac,
						   1024,
						   (s16 *) buf[current]);

      if(frames_read == 0) {
	break;
      }

      audio_play(&audio, buf[current], frames_read);
      current = 1 - current;
    }
  } break;
  case FORMAT_VORBIS: {
    while(mem.pos < mem.len) {
      f32 **output;
      s32 channels, samples;
      s32 consumed = stb_vorbis_decode_frame_pushdata(context.vorbis,
						      mem.data + mem.pos,
						      mem.len - mem.pos,
						      &channels,
						      &output,
						      &samples);
      if(samples > 0) {
	if(consumed > 0) {

	  for(s32 i=0;i<samples;i++) {
	    memcpy((f32 *) buf[current] + i*2 + 0, &output[0][i], sizeof(f32));
	    memcpy((f32 *) buf[current] + i*2 + 1, &output[1][i], sizeof(f32));
	  }

	  audio_play(&audio, buf[current], samples);
	  current = 1 - current;

	} else {
	  UNREACHABLE();
	}
      } else {
	if(consumed > 0) {
	  // ignore
	} else {
	  // insufficient data
	  break;
	}
      }

      mem.pos += consumed;
    }
  } break;
  case FORMAT_MPG: {
    plm_samples_t *samples;
    do {
      samples = plm_decode_audio(context.mpg);
      memcpy(buf[current], samples->interleaved, samples->count * 2 * sizeof(f32));
      audio_play(&audio, buf[current], samples->count);
      current = 1 - current;
    }while(samples);

  } break;
  case FORMAT_AAC: {
    while(mem.pos < mem.len) {
      Aac_Error error = aac_decode(&context.aac,
				   mem.data + mem.pos,
				   mem.len - mem.pos,
				   buf[current]);
      if(error != AAC_ERROR_NONE) {
	fprintf(stderr, "ERROR: error happened decoding the aac-data: %s\n", aac_error_name(error));
	return 1;
      }

      audio_play(&audio, buf[current], 1024);
      current = 1 - current;

      mem.pos += context.aac.consumed;
    }

  } break;
  case FORMAT_MP4_AAC: {

   if(aac_set_raw_block_params(&context.mp4.aac,
                              context.mp4.dec.as.audio.channels,
                              context.mp4.dec.as.audio.sample_rate) != AAC_ERROR_NONE) {
    TODO();
  }

    int keep_reading = 1;
    while(keep_reading) {
      u64 remaining = sizeof(cache) - cache_len;
      while(0 < remaining && keep_reading) {

	u64 read;
	if(mp4_decoder_read(&context.mp4.dec, cache + cache_len, remaining, &read)) {
	  cache_len += read;
	  remaining = sizeof(cache) - cache_len;
	} else {
	  keep_reading = 0;
	}

      }

    Aac_Error error = aac_decode(&context.mp4.aac, cache, cache_len, buf[current]);
    if(error != AAC_ERROR_NONE) {
      fprintf(stderr, "ERROR: error happened decoding aac-data: %s\n", aac_error_name(error));
      return 1;
    }
    cache_len -= context.mp4.aac.consumed;
    memmove(cache, cache + context.mp4.aac.consumed, cache_len);

    audio_play(&audio, buf[current], 1024);
    current = 1 - current;

    }
} break;
  }

  audio_block(&audio);
  audio_free(&audio);

  return 0;
}
