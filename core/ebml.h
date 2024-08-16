#ifndef EBML_H
#define EBML_H

// MIT License
//
// Copyright (c) 2024 Justin Schartner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//https://datatracker.ietf.org/doc/rfc8794/
//https://www.matroska.org/technical/elements.html

/*
   +==============+=============+===============================+
   | Octet Length | Usable Bits | Representation                |
   +==============+=============+===============================+
   | 1            | 7           | 1xxx xxxx                     |
   +--------------+-------------+-------------------------------+
   | 2            | 14          | 01xx xxxx xxxx xxxx           |
   +--------------+-------------+-------------------------------+
   | 3            | 21          | 001x xxxx xxxx xxxx xxxx xxxx |
   +--------------+-------------+-------------------------------+
   | 4            | 28          | 0001 xxxx xxxx xxxx xxxx xxxx |
   |              |             | xxxx xxxx                     |
   +--------------+-------------+-------------------------------+
   | 5            | 35          | 0000 1xxx xxxx xxxx xxxx xxxx |
   |              |             | xxxx xxxx xxxx xxxx           |
   +--------------+-------------+-------------------------------+
 */

typedef unsigned char Ebml_u8;
typedef unsigned long long int Ebml_u64;
#define u8 Ebml_u8
#define u64 Ebml_u64

#ifndef EBML_DEF
#  define EBML_DEF static inline
#endif // EBML_DEF

#ifndef EBML_LOG
#  ifdef EBML_QUIET
#    define EBML_LOG(...)
#  else
#    include <stdio.h>
#    define EBML_LOG(...) fprintf(stderr, "EBML_LOG: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#  endif // EBML_QUIET
#endif // EBML_QUIET

typedef enum{
  EBML_TYPE_MASTER,
  EBML_TYPE_UINT,
  EBML_TYPE_STRING,
  EBML_TYPE_BINARY,
  EBML_TYPE_UTF8,
  EBML_TYPE_FLOAT,
}Ebml_Type;

#define EBML_TABLE_BASE				\
  EBML_ENTRY(0x1A45DFA3, MASTER, EBML)		\
  EBML_ENTRY(0x4286, UINT, EBMLVersion)		\
  EBML_ENTRY(0x42F7, UINT, EBMLReadVersion)	\
  EBML_ENTRY(0x42f2, UINT, EBMLMaxIDLength)	\
  EBML_ENTRY(0x42f3, UINT, EBMLMaxSizeLength)	\
  EBML_ENTRY(0x4282, STRING, DocType)		\
  EBML_ENTRY(0x4287, UINT, DocTypeVersion)	\
  EBML_ENTRY(0x4285, UINT, DocTypeReadVersion)	\
  EBML_ENTRY(0xbf, BINARY, CRC_32)		\
  EBML_ENTRY(0xec, BINARY, Void)		\

#define EBML_TABLE_MKV				\
  EBML_ENTRY(0x18538067, MASTER, Segment)	\
  EBML_ENTRY(0x114d9b74, MASTER, SeekHead)	\
  EBML_ENTRY(0x4DBB, MASTER, Seek)		\
  EBML_ENTRY(0x53AB, BINARY, SeekID)		\
  EBML_ENTRY(0x53AC, UINT, SeekPosition)	\
  EBML_ENTRY(0x1549A966, MASTER, Info)		\
  EBML_ENTRY(0x2AD7B1, UINT, TimestampScale)	\
  EBML_ENTRY(0x1654AE6B, MASTER, Tracks)	\
  EBML_ENTRY(0x4D80, UTF8, MuxingApp)		\
  EBML_ENTRY(0x5741, UTF8, WritingApp)		\
  EBML_ENTRY(0xAE, MASTER, TrackEntry)		\
  EBML_ENTRY(0x1254C367, MASTER, Tags)		\
  EBML_ENTRY(0x73A4, BINARY, SegmentUUID)	\
  EBML_ENTRY(0xD7, UINT, TrackNumber)		\
  EBML_ENTRY(0x7373, MASTER, Tag)		\
  EBML_ENTRY(0x1F43B675, MASTER, Cluster)	\
  EBML_ENTRY(0x4489, FLOAT, Duration)		\
  EBML_ENTRY(0x73C5, UINT, TrackUID)		\
  EBML_ENTRY(0x63C0, MASTER, Targets)		\
  EBML_ENTRY(0xE7, UINT, Timestamp)		\
  EBML_ENTRY(0x9C, UINT, FlagLacing)		\
  EBML_ENTRY(0x67C8, MASTER, SimpleTag)		\
  EBML_ENTRY(0x63C5, UINT, TagTrackUID)		\
  EBML_ENTRY(0xA3, BINARY, SimpleBlock)		\
  EBML_ENTRY(0x1C53BB6B, MASTER, Cues)		\
  EBML_ENTRY(0x22B59C, STRING, Language)	\
  EBML_ENTRY(0x86, STRING, CodecID)		\
  EBML_ENTRY(0x45A3, UTF8, TagName)		\
  EBML_ENTRY(0xBB, MASTER, CuePoint)		\
  EBML_ENTRY(0x83, UINT, TrackType)		\
  EBML_ENTRY(0x4487, UTF8, TagString)		\
  EBML_ENTRY(0xB3, UINT, CueTime)		\
  EBML_ENTRY(0xB7, MASTER, CueTrackPositions)	\
  EBML_ENTRY(0xF7, UINT, CueTrack)		\
  EBML_ENTRY(0xF1, UINT, CueClusterPosition)	\
  EBML_ENTRY(0xF0, UINT, CueRelativePosition)	\
  EBML_ENTRY(0xE1, MASTER, Audio)		\
  EBML_ENTRY(0x9F, UINT, Channels)		\
  EBML_ENTRY(0x63A2, BINARY, CodecPrivate)	\
  EBML_ENTRY(0xB5, FLOAT, SamplingFrequency)	\
  EBML_ENTRY(0x6264, UINT, BitDepth)		\

#ifndef EBML_TABLE

#  ifdef EBML_TABLE_ADD_MKV
#    define EBML_TABLE \
  EBML_TABLE_BASE \
  EBML_TABLE_MKV

#  else
#    define EBML_TABLE EBML_TABLE_BASE

#  endif // EBML_TABLE_ADD_MKV

#endif // EBML_TABLE

typedef enum {

#define EBML_ENTRY(id, type, name) EBML_ID_##name = id ,
  EBML_TABLE
#undef EBML_ENTRY

}Ebml_Id;

typedef struct{
  Ebml_Type type;
  Ebml_Id id;
}Ebml_Elem;

typedef struct{
  u8 *data;
  u64 len;
}Ebml;

#define ebml_from(d, l) (Ebml) {(d), (l)}

EBML_DEF int ebml_next(Ebml *e, u64 *size, Ebml_Elem *elem);
EBML_DEF const char *ebml_id_name(Ebml_Id id);
EBML_DEF const char *ebml_type_name(Ebml_Type type);

typedef struct {
  u64 data;
  u8 len;
} Ebml_Vint;

EBML_DEF Ebml_Vint ebml_to_vint(u64 n);

#ifdef EBML_IMPLEMENTATION

EBML_DEF int ebml_next(Ebml *e, u64 *size, Ebml_Elem *elem) {

  // READ id
  if(e->len == 0) {
    return 0;
  }

  u8 b = *e->data;

  u8 bit = 1 << 7;
  u8 i=1;
  for(;i<=8;i++) {
    if(b & bit) break;
    bit >>= 1;
  }

  if(e->len < i) {
    return 0;
  }

  u64 id = *e->data;
  for(u8 j=1;j<i;j++) {
    id <<= 8;
    id += e->data[j];
  }
  e->data += i;
  e->len  -= i;

  switch(id) {
#define EBML_ENTRY(ebml_id, ebml_type, ebml_name)	\
    case (ebml_id): {					\
      elem->id = (EBML_ID_##ebml_name);			\
      elem->type = (EBML_TYPE_##ebml_type);		\
    } break;
    EBML_TABLE
#undef EBML_ENTRY
  default: {
      EBML_LOG("Unknown id: 0x%llX", id);
      return 0;
    } break;
  }

  //READ size
  if(e->len == 0) {
    return 0;
  }

  b = *e->data;

  bit = 1 << 7;
  i=1;
  for(;i<=8;i++) {
    if(b & bit) break;
    bit >>= 1;
  }

  if(e->len < i) {
    return 0;
  }

  *size = *e->data & ~bit;
  for(u8 j=1;j<i;j++) {
    *size <<= 8;
    *size += e->data[j];
  }
  e->data += i;
  e->len  -= i;

  // LOOKUP id

  e->data += *size;
  e->len  -= *size;

  return 1;
}

EBML_DEF const char *ebml_id_name(Ebml_Id id) {

  switch(id) {
#define EBML_ENTRY(ebml_id, ebml_type, ebml_name) case ebml_id : return #ebml_name;
    EBML_TABLE
#undef EBML_ENTRY
  }

  return NULL;
}

EBML_DEF const char *ebml_type_name(Ebml_Type type) {
  switch(type) {
  case EBML_TYPE_MASTER: return "Master Element";
  case EBML_TYPE_UINT: return "Unsigned Integer";
  case EBML_TYPE_STRING: return "String";
  case EBML_TYPE_BINARY: return "Binary";
  case EBML_TYPE_UTF8: return "Utf-8";
  case EBML_TYPE_FLOAT: return "Float";
  }

  return NULL;
}

EBML_DEF Ebml_Vint ebml_to_vint(u64 victim) {

  Ebml_Vint v;

  if(victim < 0x80) {
    v.data = 0x80 | victim;
    v.len = 1;
  } else if(victim < 0x4000) {
		u64 r = victim & 0xff;
		u64 g = (victim & 0xff00) >> 8;
    v.data = (r << 8) | 0x40 | g;
    v.len = 2;
  } else if(victim < 0x300000) {
		u64 r = victim & 0xff;
		u64 g = (victim & 0xff00) >> 8;
		u64 b = (victim & 0xff0000) >> 16;
    v.data = (r << 16) | (g << 8) | 0x30| b;
    v.len = 3;
  } else if(victim < 0x10000000) {
		u64 r = victim & 0xff;
		u64 g = (victim & 0xff00) >> 8;
		u64 b = (victim & 0xff0000) >> 16;
		u64 a = (victim & 0xff000000) >> 24;
    v.data = (r << 24) | (g << 16) | (b << 8) | 0x10 | a;
    v.len = 4;
  } else if(victim < 0x800000000) {
		u64 r = victim & 0xff;
		u64 g = (victim & 0xff00) >> 8;
		u64 b = (victim & 0xff0000) >> 16;
		u64 a = (victim & 0xff000000) >> 24;
		u64 f = (victim & 0xff00000000) >> 32;
    v.data = (r << 32) | (g << 24) | (b << 16) | (a << 8) | 0x80 | f;
    v.len = 5;
  } else {
    v.len = 0;
  }

  return v;
}

#endif // EBML_IMPLEMENTATION

#undef u8
#undef u64

#endif // EBML_H
