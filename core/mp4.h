#ifndef MP4_H_
#define MP4_H_

#include <assert.h>
#include <stdarg.h>

// https://web.archive.org/web/20180219054429/http://l.web.umkc.edu/lizhu/teaching/2016sp.video-communication/ref/mp4.pdf

#ifndef MP4_DEF
#  define MP4_DEF static inline
#endif // MP4_DEF

typedef unsigned char Mp4_u8;
typedef unsigned short Mp4_u16;
typedef unsigned int Mp4_u32;
typedef unsigned long long int Mp4_u64;

#define u8 Mp4_u8
#define u16 Mp4_u16
#define u32 Mp4_u32
#define u64 Mp4_u64

MP4_DEF u16 mp4_swap_u16(u16 in);
MP4_DEF u32 mp4_swap_u32(u32 in);

#define MP4_TYPE_XS				\
  MP4_TYPE_X(ftyp)				\
       MP4_TYPE_X(moov)				\
       MP4_TYPE_X(trak)				\
       MP4_TYPE_X(mdia)				\
       MP4_TYPE_X(hdlr)				\
       MP4_TYPE_X(minf)				\
       MP4_TYPE_X(stbl)				\
       MP4_TYPE_X(stsd)				\
       MP4_TYPE_X(tkhd)				\
       MP4_TYPE_X(moof)				\
       MP4_TYPE_X(traf)				\
       MP4_TYPE_X(tfhd)				\
       MP4_TYPE_X(mdat)				\

typedef enum {
  MP4_TYPE_NONE = 0,
#define MP4_TYPE_X(t) MP4_TYPE_##t,
  MP4_TYPE_XS
#undef MP4_TYPE_X
} Mp4_Type;

MP4_DEF int mp4_type_parse(u8 buf[4], Mp4_Type *type);

#define MP4_CODEC_XS				\
  MP4_CODEC_X(mp4a)

typedef enum {
  MP4_CODEC_NONE = 0,
#define MP4_CODEC_X(t) MP4_CODEC_##t ,
  MP4_CODEC_XS
#undef MP4_CODEC_X
} Mp4_Codec;

static char *MP4_CODEC_CSTR[] = {
  NULL,
#define MP4_CODEC_X(t) #t ,
  MP4_CODEC_XS
#undef MP4_CODEC_X
};

MP4_DEF int mp4_codec_parse(u8 buf[4], Mp4_Codec *codec);

typedef int (*Mp4_Read)(void *opaque, u8 *data, u64 len);
typedef int (*Mp4_Seek)(void *opaque, u64 offset);

typedef struct {
  u16 channels;
  u32 sample_rate;
  u16 sample_size;
} Mp4_Audio;

typedef enum {
  MP4_MEDIA_TYPE_AUDIO,
  MP4_MEDIA_TYPE_VIDEO,
}Mp4_Media_Type;

typedef struct {
  // input
  Mp4_Read read;
  Mp4_Seek seek;
  void *opaque;
  u64 off;

  // output
  u32 track_id;
  Mp4_Codec codec;
  Mp4_Media_Type type;
  union {
    Mp4_Audio audio;
  } as;

  // internal
  u64 mdat_len;
  u64 prev_size;
  u64 prev_off;
} Mp4_Decoder;

MP4_DEF int mp4_decoder_find(Mp4_Decoder *m, Mp4_Type needle);
#define mp4_decoder_query(m, ...) mp4_decoder_query_impl((m), __VA_ARGS__, MP4_TYPE_NONE)
MP4_DEF int mp4_decoder_query_impl(Mp4_Decoder *m, ...);
MP4_DEF int mp4_decoder_open(Mp4_Decoder *m,
			     void *opaque,
			     Mp4_Read read,
			     Mp4_Seek seek);
MP4_DEF int mp4_decoder_read(Mp4_Decoder *m, u8 *buf, u64 len, u64 *read);


#ifdef MP4_IMPLEMENTATION

MP4_DEF u16 mp4_swap_u16(u16 in) {
  return
    ((in & 0x00ff) << 8) |
    ((in & 0xff00) >> 8);
}

MP4_DEF u32 mp4_swap_u32(u32 in) {
  return
    ((in & 0x000000ff) << 24) |
    ((in & 0x0000ff00) << 8) |
    ((in & 0x00ff0000) >> 8) |
    ((in & 0xff000000) >> 24);
}

MP4_DEF int mp4_type_parse(u8 buf[4], Mp4_Type *type) {

#define MP4_TYPE_X(t) do{			\
    if(memcmp(buf, #t, 4) == 0) {		\
      *type = MP4_TYPE_##t ;			\
      return 1;					\
    }						\
  }while(0);
  MP4_TYPE_XS
#undef MP4_TYPE_X
  
    return 0;
}

MP4_DEF int mp4_codec_parse(u8 buf[4], Mp4_Codec *codec) {

#define MP4_CODEC_X(t) do{			\
    if(memcmp(buf, #t, 4) == 0) {		\
      *codec = MP4_CODEC_##t ;			\
      return 1;					\
    }						\
  }while(0);
  MP4_CODEC_XS
#undef MP4_CODEC_X
  
    return 0;
}

MP4_DEF int mp4_decoder_find(Mp4_Decoder *m, Mp4_Type needle) {
  u8 buf[8];
  
  while(1) {
    if(!m->read(m->opaque, buf, 8)) {
      break;
    }
    m->off += 8;

    m->prev_size = mp4_swap_u32(*(u32 *) buf);
    // printf("(%llu) - %u bytes - %.*s\n", m->off, m->prev_size, 4, buf + 4); fflush(stdout);

    Mp4_Type type;
    if(mp4_type_parse(buf + 4, &type)) {
      if(type == needle) {
	return 1;
      }
    }

    u64 seek_to = m->off - 8 + m->prev_size;
    if(!m->seek(m->opaque, seek_to)) {
      return 0;
    }
    m->off = seek_to;
  }  

  return 0;
}

MP4_DEF int mp4_decoder_query_impl(Mp4_Decoder *m, ...) {
  va_list ts;
  va_start(ts, m);

  while(1) {

    Mp4_Type type = va_arg(ts, Mp4_Type);
    if(type == MP4_TYPE_NONE) {
      return 1;
    }

    if(!mp4_decoder_find(m, type)) {
      return 0;
    }
    
  }

  va_end(ts);
  
  return 1;
}

MP4_DEF int mp4_decoder_open(Mp4_Decoder *m,
			     void *opaque,
			     Mp4_Read read,
			     Mp4_Seek seek) {
  m->opaque = opaque;
  m->read = read;
  m->seek = seek;
  m->off = 0;
  m->mdat_len = 0;
  m->prev_off = 0;

  u8 buf[1024];

  if(!mp4_decoder_query(m,
			MP4_TYPE_moov,
			MP4_TYPE_trak)) {
    return 0;
  }
  u64 trak = m->off;

  {
    if(!mp4_decoder_query(m, MP4_TYPE_tkhd)) {
      return 0;
    }

    u64 tkhd_read = 0
      + 4 /* TrackHeaderBox.(version + flags) */
      + 8 /* TrackHeaderBox.creation_time */
      + 8 /* TrackHeaderBox.modification_time */
      + 4 /* TrackHeaderBox.track_id */
      ;
    assert(tkhd_read < sizeof(buf));

    if(!m->read(m->opaque, buf, tkhd_read)) {
      return 0;
    }
    m->off = tkhd_read;

    if(buf[0] == 1) {
      m->track_id = mp4_swap_u32(*(u32*) (buf + 20));
    } else {
      m->track_id = mp4_swap_u32(*(u32*) (buf + 16));
    }    
  
    if(!m->seek(m->opaque, trak)) {
      return 0;
    }
    m->off = trak;
  }
  
  if(!mp4_decoder_query(m, MP4_TYPE_mdia)) {
    return 0;
  }
  u64 mdia = m->off;

  if(!mp4_decoder_query(m, MP4_TYPE_hdlr)) {
    return 0;
  }
  
  if(!m->read(m->opaque, buf, 12)) {
    return 0;
  }
  m->off += 12;

  if(memcmp(buf + 8, "soun", 4) == 0) {
    m->type = MP4_MEDIA_TYPE_AUDIO;
  } else if(memcmp(buf + 8, "vide", 4) == 0) {
    m->type = MP4_MEDIA_TYPE_VIDEO;
  } else {
    return 0;
  }

  if(!m->seek(m->opaque, mdia)) {
    return 0;
  }
  m->off = mdia;

  if(!mp4_decoder_query(m,
			MP4_TYPE_minf,
			MP4_TYPE_stbl,
			MP4_TYPE_stsd)) {
    return 0;
  }

  if(m->type == MP4_MEDIA_TYPE_AUDIO) {
    const u64 stsd_read = 0
      + 4 /* SampleDescriptionBox.(version + flags) */
      + 4 /* SampleDescriptionBox.entry_count */
      + 4 /* SampleEntry.size */
      + 4 /* SampleEntry.type */
      + 6 /* SampleEntry.reserved */
      + 2 /* SampleEntry.data_reference_index */
      + 8 /* AudioSampleEntry.reserved */
      + 2 /* AudioSampleEntry.channelcount */
      + 2 /* AudioSampleEntry.samplesize */
      + 2 /* AudioSampleEntry.pre_defined */
      + 2 /* AudioSampleEntry.reserved */
      + 4 /* AudioSampleEntry.samplerate */
      ;
    assert(stsd_read < sizeof(buf));

    if(!m->read(m->opaque, buf, stsd_read)) {
      return 0;
    }
    m->off += stsd_read;
    u32 entry_count = mp4_swap_u32(*(u32 *) (buf + 4));
    if(entry_count != 1) {
      return 0; // TODO
    }

    if(!mp4_codec_parse(buf + 12, &m->codec)) {
      return 0;
    }

    m->as.audio = (Mp4_Audio) {
      .channels = mp4_swap_u16(*(u16 *) (buf + 32)),
      .sample_size = mp4_swap_u16(*(u16 *) (buf + 34)),
      .sample_rate = mp4_swap_u32(*(u32 *) (buf + 38)),
    };
    
  } else {
    return 1; //TODO
    
  }
  
  if(!m->seek(m->opaque, 0)) {
    return 0;
  }
  m->off = 0;
  
  return 1;
}

MP4_DEF int mp4_decoder_read(Mp4_Decoder *m, u8 *buf, u64 len, u64 *read) {

  if(m->mdat_len == 0) {
    u64 off = m->off;

    if(!mp4_decoder_query(m,
			  MP4_TYPE_moof,
			  MP4_TYPE_traf,
			  MP4_TYPE_tfhd)) {
      return 0;
    }

    u8 buf[8];
    if(!m->read(m->opaque, buf, 8)) {
      return 0;
    }
    m->off += 8;
    // TODO: verifiy track_id

    if(!m->seek(m->opaque, off)) {
      return 0;
    }
    m->off = off;
    
    if(!mp4_decoder_query(m, MP4_TYPE_mdat)) {
      return 0;
    }
    m->mdat_len = m->prev_size - 8;    
    m->prev_off = m->off + m->mdat_len;
    
  }

  if(m->mdat_len == 0) {
    return 0;
  } else {
    u64 to_read = len;
    if(to_read > m->mdat_len) {
      to_read = m->mdat_len;
    }

    if(!m->read(m->opaque, buf, to_read)) {
      return 0;
    }
    *read = to_read;
    m->mdat_len -= to_read;

    if(m->mdat_len == 0) {

      if(!m->seek(m->opaque, m->prev_off)) {
	return 0;
      }
      m->off = m->prev_off;
    }

    return 1;
  }
  
}

#endif // MP4_IMPLEMENTATION

#undef u8
#undef u16
#undef u32
#undef u64

#endif // MP4_H_

