
#define WAV_IMPLEMENTATION
#include <core/wav.h>

#define AUDIO_IMPLEMENTATION
#include <core/audio.h>

#define IO_IMPLEMENTATION
#include <core/io.h>

#define MINIMP3_IMPLEMENTATION
#include <thirdparty/minimp3.h>

#include <core/types.h>

int wav_io_read(void *opaque, u8 *data, u64 len) {

  u64 read;
  Io_Error error = io_file_read((Io_File *) opaque,
				data,
				len,
				&read);

  if(error != IO_ERROR_NONE ||
     read != len) {
    return 0;
  }

  return 1;
}

int wav_io_seek(void *opaque, u64 offset) {
  return io_file_seek((Io_File *) opaque, offset) == IO_ERROR_NONE;
}

int main(s32 argc, char **argv) {

  if(argc < 2) {
    fprintf(stderr, "ERROR: Please provide an argument\n");
    fprintf(stderr, "USAGE: %s <audio-file>\n", argv[0]);
    fflush(stderr);
    return 1;
  }
  char *filepath = argv[1];

  Io_File file;
  Io_Error error = io_file_ropenc(&file, filepath);
  switch(error) {
  case IO_ERROR_NONE:
    // pass
    break;
  case IO_ERROR_FILE_NOT_FOUND:
    fprintf(stderr, "ERROR: Cannot find file: '%s'\n", filepath);
    return 1;
    break;
  default:
    panic("Unhandled io_error: %d\n", error);
    break;
  }

  typedef enum {
    KIND_UNKNOWN = 0,
    KIND_WAV,
    KIND_MP3,
  } Kind;

  Kind kind = KIND_UNKNOWN;
  s16 sample_buf[2][4096 * 2 * 4];
  s32 current_sample_buf = 0;

  Wav_Decoder decoder;
  if(wav_decoder_open(&decoder,
		      &file,
		      wav_io_read,
		      wav_io_seek)) {
    kind = KIND_WAV;
  } else {
    
    if(io_file_seek(&file, 0) != IO_ERROR_NONE) {
      TODO();
    }
    
  }
  
  u8 buf[1024];
  u64 buf_len = 0;  
  if(io_file_read(&file, buf, sizeof(buf), &buf_len) != IO_ERROR_NONE) {
    TODO();
  }

  mp3dec_t mp3d;
  mp3dec_init(&mp3d);
  mp3dec_frame_info_t info;
  s32 mp3_samples = mp3dec_decode_frame(&mp3d, buf, (s32) buf_len, sample_buf[current_sample_buf], &info);
  if(mp3_samples > 0 && info.frame_bytes > 0) {
    // Found mp3 sample in file
    kind = KIND_MP3;
  } 

  Audio_Fmt fmt = 0;
  int channels = 0;
  int sample_rate = 0;
  switch(kind) {
  case KIND_UNKNOWN:
    fprintf(stderr, "ERROR: Unrecognized file format. Currently supported is WAV and MP3\n");
    return 1;
    //break;
  case KIND_WAV:
    if(decoder.format == WAV_FORMAT_PCM) {
      fmt = AUDIO_FMT_S16;
    } else if(decoder.format == WAV_FORMAT_IEEE) {
      fmt = AUDIO_FMT_FLT;
    } else {
      TODO();
    }

    channels = decoder.channels;
    sample_rate = decoder.sample_rate;
    break;
  case KIND_MP3:
    buf_len = 0;
    if(io_file_seek(&file, 0) != IO_ERROR_NONE) {
      TODO();
    }
    
    fmt = AUDIO_FMT_S16;
    channels = info.channels;
    sample_rate = info.hz;
    break;
  }

  Audio audio;
  if(!audio_init(&audio, fmt, channels, sample_rate)) {
    TODO();
  }
  
  u32 samples = 0;
  while(1) {

    int stop_decoding = 0;

    switch(kind) {
    case KIND_UNKNOWN:
      UNREACHABLE();
    case KIND_WAV:
      if(!wav_decoder_decode(&decoder,
			     (u8 *) sample_buf[current_sample_buf],
			     sizeof(sample_buf[current_sample_buf]),
			     &samples)) {
	stop_decoding = 1;
      }
      break;
    case KIND_MP3:
      
      u64 to_read = sizeof(buf) - buf_len;
      u64 read;
      if(io_file_read(&file, buf + buf_len, to_read, &read) != IO_ERROR_NONE) {
	TODO();
      }
      buf_len += read;

      samples = (u32) mp3dec_decode_frame(&mp3d,
					  buf,
					  (s32) buf_len,
					  sample_buf[current_sample_buf],
					  &info);
      if(samples == 0) {
	stop_decoding = 1;
      }
      
      buf_len -= (u64) info.frame_bytes;
      memmove(buf, buf + info.frame_bytes, buf_len);
      
      break;
    }

    if(stop_decoding) {
      break;
    }

    audio_play(&audio, (u8 *) sample_buf[current_sample_buf], samples);
    current_sample_buf = 1 - current_sample_buf;

  }

  audio_block(&audio);
  audio_free(&audio);

  io_file_close(&file);

  return 0;
}
