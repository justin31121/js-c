#ifndef AAC_H
#define AAC_H

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

// TODO: rewrite Aac_Bits and the functions using it, to check
//       the bounds once per function and not on every Aac_Bits.read-call
// TODO: Aac_Ics_Info is selected in a lot of functions ?, why not just pass them ?
// NOTE: maybe add 'static inline' to functions without allowing to custmoize via AAC_DEF
// TODO: AAC_SFB_BAND_TOTAL_SHORT to AAC_SF_BAND_TOTAL_SHORT

typedef unsigned char Aac_u8;
typedef signed char Aac_s8;
typedef unsigned short Aac_u16;
typedef short Aac_s16;
typedef unsigned int Aac_u32;
typedef int Aac_s32;
typedef unsigned long long int Aac_u64;
typedef long long int Aac_s64;

#define u8 Aac_u8
#define s8 Aac_s8
#define u16 Aac_u16
#define s16 Aac_s16
#define u32 Aac_u32
#define s32 Aac_s32
#define u64 Aac_u64
#define s64 Aac_s64

#ifndef AAC_DEF
#  define AAC_DEF static inline
#endif // AAC_DEF

typedef union {
  s64 w64;
  struct {
    u32 lo32;
    s32 hi32;
  } r;
} Aac_Word;

AAC_DEF s32 aac_clz(s32 x);
AAC_DEF s32 aac_fastabs(s32 x);
AAC_DEF s32 aac_mulshift_32(s32 x, s32 y);
AAC_DEF s16 aac_clip_to_short(s32 x);
AAC_DEF u32 aac_rand_u32(u32 *last);
AAC_DEF s32 aac_inverse_square_root(s32 x);
AAC_DEF s32 aac_min(s32 a, s32 b);
AAC_DEF s32 aac_max(s32 a, s32 b);
AAC_DEF s64 aac_madd_64(s64 sum64, s32 x, s32 y);

#define AAC_ERRORS_X						\
  AAC_ERROR_X(AAC_ERROR_UNREACHABLE)				\
       AAC_ERROR_X(AAC_ERROR_NONE)				\
       AAC_ERROR_X(AAC_ERROR_NOT_ENOUGH_DATA)			\
       AAC_ERROR_X(AAC_ERROR_UNKNOWN_CHANNEL_CONFIGURATION)	\
       AAC_ERROR_X(AAC_ERROR_UNKNOWN_SAMPLING_FREQUENCY_INDEX)	\
       AAC_ERROR_X(AAC_ERROR_UNKNOWN_BLOCK_ID)			\
       AAC_ERROR_X(AAC_ERROR_CHANNELS_TOO_HIGH)

typedef enum {
#define AAC_ERROR_X(name) name ,
  AAC_ERRORS_X
#undef AAC_ERROR_X
} Aac_Error;

AAC_DEF char *aac_error_name(Aac_Error error);

#define AAC_MAX_HUFF_BITS 20

typedef struct {
  s32 max_bits;
  u8 count[AAC_MAX_HUFF_BITS];
  s32 offset;
} Aac_Huff_Info;

AAC_DEF s32 aac_decode_huffman_scalar(s16 *huff_tab,
				     Aac_Huff_Info *huff_tab_info,
				     u32 bit_buf,
				     s32 *value);

typedef struct {
  u8 *data;
  u64 len;
  u8 byte;
  s8 i;
} Aac_Bits;

#define aac_bits_fmt "{ data::%p, len:%llu, byte:%u, i::%d}"
#define aac_bits_arg(b) (void *) (b).data, (b).len, (b).byte, (b).i

#define aac_bits_from(d, l) (Aac_Bits) { .data = (d), .len = (l), .i = (-1) }
#define aac_bits_are_byte_aligned(b) ((b).i == -1)

#define aac_bits_next_declaration(prefix, size)				\
  AAC_DEF Aac_Error aac_bits_next_##prefix##size (Aac_Bits *b, u8 num_of_bits, prefix##size *out);

aac_bits_next_declaration(u, 8)
aac_bits_next_declaration(s, 8)
aac_bits_next_declaration(u, 16)
aac_bits_next_declaration(u, 32)
aac_bits_next_declaration(s, 32)
aac_bits_next_declaration(u, 64)

#define aac_bits_peek_declaration(prefix, size)				\
  AAC_DEF Aac_Error aac_bits_peek_##prefix##size (Aac_Bits *b, u8 num_of_bits, prefix##size *out);

aac_bits_peek_declaration(u, 32)

AAC_DEF Aac_Error aac_bits_decode_one_scale_factor(Aac_Bits *b, s32 *value);
AAC_DEF Aac_Error aac_bits_discard(Aac_Bits *b, u64 n);

#define AAC_PROFILE_MP 0
#define AAC_PROFILE_LC 1
#define AAC_PROFILE_SSR	2

#define AAC_MAX_NCHANS 2

#define AAC_MAX_NCHANS_ELEM 2
#define AAC_MAX_WIN_GROUPS 8
#define AAC_MAX_SFB_SHORT 15
#define AAC_MAX_SF_BANDS (AAC_MAX_SFB_SHORT*AAC_MAX_WIN_GROUPS)
#define AAC_MAX_MS_MASK_BYTES ((AAC_MAX_SF_BANDS + 7) >> 3)
#define AAC_MAX_TNS_FILTERS 8
#define AAC_MAX_TNS_ORDER 20
#define AAC_MAX_TNS_COEFS 60
#define AAC_MAX_NSAMPS 1024
#define AAC_FILL_BUF_SIZE 269
#define AAC_DATA_BUF_SIZE 510
#define AAC_MAX_NUM_PCE_ADIF 16
#define AAC_MAX_GAIN_ADJUST 7
#define AAC_MAX_GAIN_BANDS 3
#define AAC_MAX_GAIN_WIN 8
#define AAC_MAX_PULSES 4
#define AAC_MAX_PRED_SFB 41
#define AAC_MAX_COMMENT_BYTES 255
#define AAC_ADIF_COPYID_SIZE 9

#define AAC_MAX_NUM_FCE 15
#define AAC_MAX_NUM_SCE 15
#define AAC_MAX_NUM_BCE 15
#define AAC_MAX_NUM_LCE  3
#define AAC_MAX_NUM_ADE  7
#define AAC_MAX_NUM_CCE 15

#define AAC_NUM_SYN_ID_BITS   3
#define AAC_NUM_INST_TAG_BITS 4

#define AAC_NUM_SAMPLE_RATES 12
#define AAC_NSAMPS_LONG 1024
#define AAC_NSAMPS_SHORT 128

#define AAC_SF_OFFSET 100
#define AAC_SF_DQ_OFFSET 15
#define AAC_FBITS_OUT_DQ 20
#define AAC_FBITS_OUT_DQ_OFF (AAC_FBITS_OUT_DQ - AAC_SF_DQ_OFFSET)
#define AAC_SQRTHALF 0x5a82799a
#define AAC_GBITS_IN_DCT4 4
#define AAC_NUM_IMDCT_SIZES 2
#define AAC_NUM_FFT_SIZES 2

#define AAC_SF_DQ_OFFSET 15
#define AAC_FBITS_OUT_DQ 20
#define AAC_FBITS_OUT_DQ_OFF (AAC_FBITS_OUT_DQ - AAC_SF_DQ_OFFSET)
#define AAC_FBITS_IN_IMDCT AAC_FBITS_OUT_DQ_OFF
#define AAC_FBITS_LOST_DCT4 1
#define AAC_FBITS_LOST_WND 1
#define AAC_FBITS_LOST_IMDCT (AAC_FBITS_LOST_DCT4 + AAC_FBITS_LOST_WND)
#define AAC_FBITS_OUT_IMDCT (AAC_FBITS_IN_IMDCT - AAC_FBITS_LOST_IMDCT)
#define AAC_RND_VAL (1 << (AAC_FBITS_OUT_IMDCT-1))

typedef struct {
  u8 id;
  u8 layer;
  u8 protect_bit;
  u8 profile;
  u8 sampling_rate_index;
  u8 private_bit;
  u8 channel_config;
  u8 original_copy;
  u8 home;

  u8 copy_bit;
  u8 copy_start;
  s32 frame_length;
  s32 buffer_full;
  u8 num_raw_data_blocks;
	
  int crc_check_word;	
} Aac_Adts_Header;

typedef struct {
  u8 copy_bit;
  u8 copy_id[AAC_ADIF_COPYID_SIZE];
  u8 original_copy;
  u8 home;
  u8 bs_type;
  s32 bit_rate;
  u8 num_pce;
  s32 buffer_full;

} Aac_Adif_Header;

typedef struct {
  u8 elem_inst_tag;
  u8 profile;
  u8 sampling_rate_index;
  u8 num_fce;
  u8 num_sce;
  u8 num_bce;
  u8 num_lce;
  u8 num_ade;
  u8 num_cce;
  u8 mono_mixdown;
  u8 stereo_mixdown;
  u8 matrix_mixdown;
  u8 fce[AAC_MAX_NUM_FCE];
  u8 sce[AAC_MAX_NUM_SCE];
  u8 bce[AAC_MAX_NUM_BCE];
  u8 lce[AAC_MAX_NUM_LCE];
  u8 ade[AAC_MAX_NUM_ADE];
  u8 cce[AAC_MAX_NUM_CCE];

  u8 comment_bytes;
  u8 comment_field[AAC_MAX_COMMENT_BYTES];
} Aac_Prog_Config_Element;

typedef struct {
  u8 ics_res_bit;
  u8 window_seq;
  u8 window_shape;
  u8 max_sfb;
  //   if(window_sequence == EIGHT_SHORT_SEQUENCE) {
  u8 sf_group;
  //   } else {
  u8 predictor_data_present;
  u8 predictor_reset;
  u8 predictor_reset_group_num;
  u8 prediction_used[AAC_MAX_PRED_SFB];
  //   }

  u8 num_window_group;
  u8 window_group_len[AAC_MAX_WIN_GROUPS];
} Aac_Ics_Info;

typedef struct {
  u8 pulse_data_present;

  u8 num_pulse;
  u8 start_sfb;
  u8 offset[AAC_MAX_PULSES];
  u8 amp[AAC_MAX_PULSES];
} Aac_Pulse_Data;

typedef struct {
  u8 tns_data_present;
  u8 num_filt[AAC_MAX_TNS_FILTERS];
  u8 coef_res[AAC_MAX_TNS_FILTERS];
  u8 length[AAC_MAX_TNS_FILTERS];
  u8 order[AAC_MAX_TNS_FILTERS];
  u8 dir[AAC_MAX_TNS_FILTERS];
  s8 coef[AAC_MAX_TNS_COEFS];
} Aac_Tns_Data;

typedef struct {
  u8 gain_control_data_present;
  u8 max_band;
  u8 adj_num[AAC_MAX_GAIN_BANDS][AAC_MAX_GAIN_WIN];
  u8 alev_code[AAC_MAX_GAIN_BANDS][AAC_MAX_GAIN_WIN][AAC_MAX_GAIN_ADJUST];
  u8 aloc_code[AAC_MAX_GAIN_BANDS][AAC_MAX_GAIN_WIN][AAC_MAX_GAIN_ADJUST];
} Aac_Gain_Control_Data;

typedef struct {
  Aac_Adts_Header adts;
  Aac_Adif_Header adif;
  Aac_Prog_Config_Element pce[AAC_MAX_NUM_PCE_ADIF];
  u8 data_buf[AAC_DATA_BUF_SIZE];
  s32 data_count;
  u8 fill_buf[AAC_FILL_BUF_SIZE];
  s32 fill_count;

  s32 channels;
  s32 use_imp_channel_map;
  s32 sampling_rate_index;

  Aac_Ics_Info ics_info[AAC_MAX_NCHANS_ELEM];

  s32 common_window;
  s16 scale_factors[AAC_MAX_NCHANS_ELEM][AAC_MAX_SF_BANDS];
  u8 sfb_code_book[AAC_MAX_NCHANS_ELEM][AAC_MAX_SF_BANDS];

  s32 ms_mask_present;
  u8 ms_mask_bits[AAC_MAX_MS_MASK_BYTES];
	
  s32 pns_used[AAC_MAX_NCHANS_ELEM];
  s32 pns_last_value;
  s32 intensity_used[AAC_MAX_NCHANS_ELEM];

  Aac_Pulse_Data pulse_data[AAC_MAX_NCHANS_ELEM];
	
  Aac_Tns_Data tns_data[AAC_MAX_NCHANS_ELEM];
  s32 tns_lpc_buf[AAC_MAX_TNS_ORDER];
  s32 tns_work_buf[AAC_MAX_TNS_ORDER];

  Aac_Gain_Control_Data gain_control_data[AAC_MAX_NCHANS_ELEM];
	
  s32 gb_current[AAC_MAX_NCHANS_ELEM];
  s32 coef[AAC_MAX_NCHANS_ELEM][AAC_MAX_NSAMPS];

  s32 overlap[AAC_MAX_NCHANS][AAC_MAX_NSAMPS];
  s32 prev_window_shape[AAC_MAX_NCHANS]; // TODO: maybe this can get to u8, since Aac_Ics_Info is completly u8
} Aac_Ps_Info_Base;

typedef enum {
  AAC_FORMAT_UNKNOWN = 0,
  
  AAC_FORMAT_ADTS    = 1,
  AAC_FORMAT_ADIF    = 2,
  AAC_FORMAT_RAW     = 3,
} Aac_Format;

// Zero initialize Aac before use
typedef struct {
  Aac_Ps_Info_Base ps_info_base;

  void *raw_sample_buf[AAC_MAX_NCHANS];
  s32 raw_sample_bytes;
  s32 raw_sample_fbits;

  u8 *fill_buf;
  s32 fill_count;
  s32 fill_ext_type;

  s32 prev_block_id;
  s32 curr_block_id;
  s32 curr_inst_tag;
  s32 sbDeinterleaveReqd[AAC_MAX_NCHANS_ELEM];
  s32 adts_blocks_left;

  s32 bit_rate;
  s32 channels;
  s32 sample_rate;
  s32 profile;
  Aac_Format format;                              // is initialized with AAC_FORMAT_UNKNOWN = 0
  s32 sbr_enabled;
  s32 tns_used;
  s32 pns_used;
  s32 frame_count;

  // output
  s32 samples;
  u64 consumed;
  
} Aac;

#include <stdio.h>
#include <stdlib.h>

#define aac_panic(...) do{						\
    fflush(stdout);						\
    fprintf(stderr, "%s:%d:ERROR: ", __FILE__, __LINE__);	\
    fflush(stderr);						\
    fprintf(stderr, __VA_ARGS__); fflush(stderr);		\
    exit(1);							\
  }while(0)

#define AAC_UNREACHABLE() aac_panic("UNREACHABLE")
#define AAC_TODO() aac_panic("TODO")

AAC_DEF Aac_Error aac_set_raw_block_params(Aac *a, s32 channels, s32 sample_rate);
AAC_DEF Aac_Error aac_decode(Aac *a, u8 *data, u64 len, u8 *output);

AAC_DEF Aac_Error aac_decode_ics(Aac_Ps_Info_Base *ps, Aac_Ics_Info *ics, Aac_Bits *b, s32 channel);
AAC_DEF Aac_Error aac_decode_spectrum_long(Aac_Ps_Info_Base *ps, Aac_Ics_Info *ics, Aac_Bits *b, s32 channel);
AAC_DEF Aac_Error aac_decode_spectrum_short(Aac_Ps_Info_Base *ps, Aac_Ics_Info *ics, Aac_Bits *b, s32 channel);

AAC_DEF Aac_Error aac_decode_noiseless_data(Aac *a, Aac_Bits *b, s32 channel);
AAC_DEF Aac_Error aac_dequantize(Aac *a, s32 channel);
AAC_DEF Aac_Error aac_stereo_process(Aac *a);
AAC_DEF Aac_Error aac_apply_pns(Aac *a, s32 channel);
AAC_DEF Aac_Error aac_deinterleave_short_blocks(Aac *a, s32 channel);
AAC_DEF Aac_Error aac_apply_tns(Aac *a, s32 channel);
AAC_DEF Aac_Error aac_apply_imdct(Aac *a, s32 channel, s32 out_channel, u8 *output);

AAC_DEF Aac_Error aac_decode_section_data(u8 *sfb_code_book,
					  Aac_Bits *b,
					  u8 window_seq,
					  u8 num_window_group,
					  u8 max_sfb);
AAC_DEF Aac_Error aac_decode_scale_factors(s16 *scale_factors,
					   Aac_Bits *b,
					   u8 *sfb_code_book,
					   u8 num_window_group,
					   u8 max_sfb,
					   s32 global_gain);
AAC_DEF Aac_Error aac_decode_pulse_data(Aac_Pulse_Data *p, Aac_Bits* b);
AAC_DEF Aac_Error aac_decode_tns_data(Aac_Tns_Data *t, Aac_Bits* b, s32 window_seq);
AAC_DEF Aac_Error aac_decode_gain_control_data(Aac_Gain_Control_Data *g, Aac_Bits *b, u8 window_seq);
AAC_DEF void aac_tns_decode_lpc_coefs(s32 *tns_lpc_buf,
				      s32 *tns_work_buf,
				      s8 *filter_coef,
				      u8 filter_res,
				      u8 order);

AAC_DEF void aac_coefs_unpack_zeros(s32 *coefs, s32 value);
AAC_DEF Aac_Error aac_coefs_unpack_quads(s32 *coefs, Aac_Bits *b, u8 cb, s32 value);
AAC_DEF Aac_Error aac_coefs_unpack_pairs_no_esc(s32 *coefs, Aac_Bits *b, u8 cb, s32 value);
AAC_DEF Aac_Error aac_coefs_unpack_pairs_esc(s32 *coefs, Aac_Bits *b, u8 cb, s32 value);
AAC_DEF s32 aac_coefs_dequantize_block(s32 *coefs, s16 width, s16 scale);
AAC_DEF void aac_coefs_stereo_process_group(s32 *coefs_left,
					    s32 *coefs_right,
					    s16 *sfb_tab,
					    s32 ms_mask_present,
					    u8 *ms_mask_ptr,
					    s32 ms_mask_offset,
					    u8 max_sfb,
					    u8 *sfb_code_book,
					    s16 *scale_factors,
					    s32* gb_current);
AAC_DEF void aac_coefs_dct4(s32 *coefs, s32 gb_current, s32 tab);
AAC_DEF void aac_coefs_dec_window_overlap(s32 *coefs,
					  s32 *overlap,
					  u8 *output,
					  s32 channels,
					  u8 window_shape,
					  s32 prev_window_shape);
AAC_DEF void aac_coefs_dec_window_overlap_long_start(s32 *coefs,
						     s32 *overlap,
						     u8 *output,
						     s32 channels,
						     u8 window_shape,
						     s32 prev_window_shape);
AAC_DEF void aac_coefs_dec_window_overlap_short(s32 *coefs,
						s32 *overlap,
						u8 *output,
						s32 channels,
						u8 window_shape,
						s32 prev_window_shape);
AAC_DEF void aac_coefs_dec_window_overlap_long_stop(s32 *coefs,
						    s32 *overlap,
						    u8 *output,
						    s32 channels,
						    u8 window_shape,
						    s32 prev_window_shape);
AAC_DEF void aac_coefs_pre_multiply_rescale(s32 *coefs, s32 es, s32 tab);
AAC_DEF void aac_coefs_bitreverse(s32 *coefs, s32 tab);
AAC_DEF void aac_coefs_r8_first_pass(s32 *x, s32 bg);
AAC_DEF void aac_coefs_r4_core(s32 *coefs, s32 bg, s32 gp, s32 *wtab);
AAC_DEF void aac_coefs_r4fft(s32 *coefs, s32 tab);
AAC_DEF void aac_coefs_post_multiply_rescale(s32 *coefs, s32 es, s32 tab);
AAC_DEF void aac_coefs_pre_multiply(s32 *coefs, s32 tab);
AAC_DEF void aac_coefs_post_multiply(s32 *coefs, s32 tab);
AAC_DEF void aac_coefs_generate_noise_vector(s32 *coefs,
						 s32 *last,
						 s32 width);
AAC_DEF void aac_coefs_copy_noise_vector(s32 *coefs_left,
					 s32 *coefs_right,
					 s32 width);
AAC_DEF s32 aac_coefs_scale_noise_vector(s32 *coefs,
					 s32 width,
					 s32 sf);
AAC_DEF s32 aac_coefs_filter_region(s32 *coefs,
				    s32 *tns_lpc_buf,
				    s32 *tns_work_buf,
				    s16 size,
				    u8 dir,
				    u8 order);

AAC_DEF Aac_Error aac_decode_next_element(Aac *a, Aac_Bits *b);
AAC_DEF Aac_Error aac_decode_fill_element(Aac *a, Aac_Bits *b);
AAC_DEF Aac_Error aac_decode_channel_pair_element(Aac *a, Aac_Bits *b);
AAC_DEF Aac_Error aac_decode_program_config_element(Aac_Prog_Config_Element *p, Aac_Bits *b);
AAC_DEF Aac_Error aac_decode_single_channel_element(Aac *a, Aac_Bits *b);
AAC_DEF Aac_Error aac_decode_lfe_channel_element(Aac *a, Aac_Bits *b);

AAC_DEF Aac_Error aac_align_to_adts_sync_word(Aac_Bits *b);
AAC_DEF Aac_Error aac_decode_adts(Aac *a, Aac_Bits *b);
AAC_DEF Aac_Error aac_calculate_implicit_adts_channel_mapping(Aac *a, Aac_Bits *b);

AAC_DEF Aac_Error aac_decode_adif(Aac *a, Aac_Bits *b);

AAC_DEF Aac_Error aac_decode_ics_info(Aac_Ics_Info *ics, Aac_Bits *b, s32 sampling_rate_index);

#ifdef AAC_IMPLEMENTATION

#define aac_clip_2n(y, n) {			\
    int sign = (y) >> 31;			\
    if (sign != ((y) >> (n)))  {		\
      (y) = sign ^ ((1 << (n)) - 1);		\
    }						\
  }

#define aac_clip_2n_shift(y, n) {		\
    int sign = (y) >> 31;			\
    if (sign != (y) >> (30 - (n)))  {		\
      (y) = sign ^ (0x3fffffff);		\
    } else {					\
      (y) = (y) << (n);				\
    }						\
  }

AAC_DEF s32 aac_clz(s32 x) {
  	s32 zeros;
	if (!x) {
		return 32;
	}

	zeros = 1;
	if (!((u32)x >> 16))	{ zeros += 16; x <<= 16; }
	if (!((u32)x >> 24))	{ zeros +=  8; x <<=  8; }
	if (!((u32)x >> 28))	{ zeros +=  4; x <<=  4; }
	if (!((u32)x >> 30))	{ zeros +=  2; x <<=  2; }

	zeros -= ((u32)x >> 31);

	return zeros;
 }

AAC_DEF s32 aac_fastabs(s32 x) {
  s32 sign = x >> (sizeof(s32) * 8 - 1);
  x ^= sign;
  x -= sign;
  return x;
}

AAC_DEF s32 aac_mulshift_32(s32 x, s32 y) {
  return (long long) x * (long long) y >> 32;
}

AAC_DEF s16 aac_clip_to_short(s32 x) {
  s32 sign;
  sign = x >> 31;
  if(sign != (x >> 15)) {
    x = sign ^ ((1 << 15) - 1);
  }
  return (s16) x;
}

AAC_DEF u32 aac_rand_u32(u32 *last) {
  u32 r = *last;
  r = (1664525U * r) + 1013904223U;
  *last = r;
  return r;
}

#define AAC_X0_COEF_2 0xc0000000
#define AAC_X0_OFF_2 0x60000000
#define AAC_Q26_3 0x0c000000
#define AAC_INVERSE_SQUARE_ROOT_ITERATIONS 4

AAC_DEF s32 aac_inverse_square_root(s32 x) {
  s32 xn = (aac_mulshift_32(x, AAC_X0_COEF_2) << 2) + AAC_X0_OFF_2;

  for(s32 i=0;i<AAC_INVERSE_SQUARE_ROOT_ITERATIONS;i++) {
    s32 t = aac_mulshift_32(xn, xn);
    t = AAC_Q26_3 - (aac_mulshift_32(x, t) << 2);
    xn = aac_mulshift_32(xn, t) << (6 - 1);
  }

  if(xn >> 30) {
    xn = (1 << 30) - 1;
  }

  return xn;
}

AAC_DEF s32 aac_min(s32 a, s32 b) {
  if(a < b) {
    return a;
  } else {
    return b;
  }
}

AAC_DEF s32 aac_max(s32 a, s32 b) {
  if(a < b) {
    return b;
  } else {
    return a;
  }
}

AAC_DEF s64 aac_madd_64(s64 sum64, s32 x, s32 y) {
  sum64 += (s64) x * (s64) y;
  return sum64;
}

#define AAC_ARRAY_LEN(arr) sizeof((arr))/sizeof(*(arr))

static char AAC_ID_ADIF[] = { 'A', 'D', 'I', 'F' };
#define ___AAC_ID_INVALID -1
#define ___AAC_ID_SCE 0
#define ___AAC_ID_CPE 1
#define ___AAC_ID_CCE 2
#define ___AAC_ID_LFE 3
#define ___AAC_ID_DSE 4
#define ___AAC_ID_PCE 5
#define ___AAC_ID_FIL 6
#define ___AAC_ID_END 7

static char *__AAC_ERROR_NAMES[] = {
#define AAC_ERROR_X(name)\
  #name ,
  AAC_ERRORS_X
#undef AAC_ERROR_X
};

AAC_DEF char *aac_error_name(Aac_Error error) {
  return __AAC_ERROR_NAMES[error];
}

#define aac_unwrap(b) do{			\
    Aac_Error __aac_unwrap_error = (b);		\
    if(__aac_unwrap_error != AAC_ERROR_NONE) {	\
      return __aac_unwrap_error;		\
    }						\
  }while(0)

#define aac_bits_next_implementation(prefix, size)			\
  AAC_DEF Aac_Error aac_bits_next_##prefix##size (Aac_Bits *b, u8 num_of_bits, prefix##size *out) { \
									\
    u##size result = 0;							\
  									\
    while(num_of_bits > 0) {						\
  									\
      if(b->i == -1) {							\
									\
	if(b->len == 0) {						\
	  return AAC_ERROR_NOT_ENOUGH_DATA;				\
	}								\
									\
	b->byte = *b->data;						\
	b->data++;							\
	b->len--;							\
	b->i = 7;							\
      }									\
									\
      u8 bit_pos = (1 << b->i);						\
      u8 bit_read = ((b->byte & bit_pos) >> b->i);			\
      result = (result << 1) | bit_read;				\
      num_of_bits--;							\
									\
      b->i--;								\
    }									\
									\
    *out = * ( prefix##size *) &result;					\
    return AAC_ERROR_NONE;						\
  }

aac_bits_next_implementation(u, 8)
aac_bits_next_implementation(s, 8)
aac_bits_next_implementation(u, 16)
aac_bits_next_implementation(u, 32)
aac_bits_next_implementation(s, 32)
aac_bits_next_implementation(u, 64)

#define aac_bits_peek_implementation(prefix, size)			\
  AAC_DEF Aac_Error aac_bits_peek_##prefix##size (Aac_Bits *b, u8 num_of_bits, prefix##size *out) { \
									\
  u##size result = 0;							\
  Aac_Bits copy = *b;							\
  									\
  while(num_of_bits > 0) {						\
  									\
    if(b->i == -1) {							\
									\
      if(b->len == 0) {							\
	return AAC_ERROR_NOT_ENOUGH_DATA;				\
      }									\
									\
      b->byte = *b->data;						\
      b->data++;							\
      b->len--;								\
      b->i = 7;								\
    }									\
									\
    u8 bit_pos = (1 << b->i);						\
    u8 bit_read = ((b->byte & bit_pos) >> b->i);			\
    result = (result << 1) | bit_read;					\
    num_of_bits--;							\
									\
    b->i--;								\
  }									\
									\
  *b = copy;								\
  *out = * ( prefix##size *) &result;					\
  return AAC_ERROR_NONE;						\
  }

aac_bits_peek_implementation(u, 32)

AAC_DEF s32 aac_decode_huffman_scalar(s16 *huff_tab,
				     Aac_Huff_Info *huff_tab_info,
				     u32 bit_buf,
				     s32 *value) {
  
  s16 *map = huff_tab + huff_tab_info->offset;
  u8 *huff_counts = huff_tab_info->count;

  u32 start = 0;
  u32 count = 0;
  u32 shift = 32;
  u32 t;
  do {
    start += count;
    start = start << 1;
    map += count;
    count = *huff_counts++;
    shift -= 1;
    t = (bit_buf >> shift) - start;
    
  } while(t >= count);

  *value = (s32) map[t];
  return (s32) (huff_counts - huff_tab_info->count); // TODO: think about this, if somehting breaks
}

static Aac_Huff_Info AAC_HUFF_SCALE_FACTOR_INFO = {
  .max_bits = 19,
  .count    = { 1,  0,  1,  3,  2,  4,  3,  5,  4,  6,  6,  6,  5,  8,  4,  7,  3,  7, 46,  0},
  .offset   = 0
};

static s16 AAC_HUFF_SCALE_FACTOR[121] = {
  0,   -1,    1,   -2,    2,   -3,    3,   -4,    4,   -5,    5,    6,   -6,    7,   -7,    8,
  -8,    9,   -9,   10,  -10,  -11,   11,   12,  -12,   13,  -13,   14,  -14,   16,   15,   17,
  18,  -15,  -17,  -16,   19,  -18,  -19,   20,  -20,   21,  -21,   22,  -22,   23,  -23,  -25,
  25,  -27,  -24,  -26,   24,  -28,   27,   29,  -30,  -29,   26,  -31,  -34,  -33,  -32,  -36,
  28,  -35,  -38,  -37,   30,  -39,  -41,  -57,  -59,  -58,  -60,   38,   39,   40,   41,   42,
  57,   37,   31,   32,   33,   34,   35,   36,   44,   51,   52,   53,   54,   55,   56,   50,
  45,   46,   47,   48,   49,   58,  -54,  -52,  -51,  -50,  -55,   43,   60,   59,  -56,  -53,
  -45,  -44,  -42,  -40,  -43,  -49,  -48,  -46,  -47,
};

AAC_DEF Aac_Error aac_bits_decode_one_scale_factor(Aac_Bits *b, s32 *value) {

  u32 bit_buf;
  aac_unwrap(aac_bits_peek_u32(b, AAC_HUFF_SCALE_FACTOR_INFO.max_bits, &bit_buf));
  bit_buf = bit_buf << (32 - AAC_HUFF_SCALE_FACTOR_INFO.max_bits);
  
  s32 bits = aac_decode_huffman_scalar(AAC_HUFF_SCALE_FACTOR,
				      &AAC_HUFF_SCALE_FACTOR_INFO,
				      bit_buf,
				      value);
  
  u64 placeholder;
  aac_unwrap(aac_bits_next_u64(b, bits, &placeholder));
  
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_bits_discard(Aac_Bits *b, u64 n) {

  while(n > 0) {

    if(b->i == -1) {
      if(b->len == 0) {
	return AAC_ERROR_NOT_ENOUGH_DATA;
      }
      
      b->byte = *b->data;
      b->data += 1;
      b->len  -= 1;
      
      b->i = 7;
    }
    
    b->i--;
    n--;
  }

  return AAC_ERROR_NONE;
}

static s32 AAC_ELEMENT_NUM_CHANNELS_TABLE[] = {
  1, 2, 0, 1, 0, 0, 0, 0
};

static s32 AAC_SAMPLE_RATE_MAP_TABLE[AAC_NUM_SAMPLE_RATES] = {
  96000, 88200, 64000, 48000, 44100, 32000, 
  24000, 22050, 16000, 12000, 11025,  8000
};

AAC_DEF Aac_Error aac_set_raw_block_params(Aac *a, s32 channels, s32 sample_rate) {
  a->format = AAC_FORMAT_RAW;

  a->profile = AAC_PROFILE_LC;
  a->ps_info_base.channels = channels;

  u8 i = 0;
  for(;i<AAC_NUM_SAMPLE_RATES;i++) {
    if(sample_rate == AAC_SAMPLE_RATE_MAP_TABLE[i]) {
      a->ps_info_base.sampling_rate_index = i;
      break;
    }
  }
  if(i == AAC_NUM_SAMPLE_RATES) {
    return AAC_ERROR_UNKNOWN_SAMPLING_FREQUENCY_INDEX;
  }

  a->channels = channels;
  a->sample_rate = AAC_SAMPLE_RATE_MAP_TABLE[a->ps_info_base.sampling_rate_index];

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode(Aac *a, u8 *data, u64 len, u8 *output) {

  Aac_Bits b = aac_bits_from(data, len);

  // Figure out format, in first pass
  if (a->format == AAC_FORMAT_UNKNOWN) {

    u32 adif_id = 0;
    aac_unwrap(aac_bits_peek_u32(&b, 32, &adif_id));
        
    if(adif_id == (*(u32 *) AAC_ID_ADIF)) {
      a->format = AAC_FORMAT_ADIF;
      aac_unwrap(aac_decode_adif(a, &b));
      
    } else {
      // assume it is ADTS by default
      a->format = AAC_FORMAT_ADTS;
      
    }
    
  }

  // If ADTS, search for next frame
  if(a->format == AAC_FORMAT_ADTS) {
    // Can have 1-4 raw data blocks per ADTS frame (header only present for first one)
    if(a->adts_blocks_left == 0) {
      aac_unwrap(aac_align_to_adts_sync_word(&b));
      aac_unwrap(aac_decode_adts(a, &b));

      if(a->channels == -1) {
	// Figure out implicit channel mapping
	aac_unwrap(aac_calculate_implicit_adts_channel_mapping(a, &b));
	
      }
      
    }
    a->adts_blocks_left--;
    
  } else if(a->format == AAC_FORMAT_RAW) {
    a->prev_block_id = ___AAC_ID_INVALID;
    a->curr_block_id = ___AAC_ID_INVALID;
    a->curr_inst_tag = ___AAC_ID_INVALID;

    a->bit_rate = 0;
    a->sbr_enabled = 0;
  }

  if(a->channels > AAC_MAX_NCHANS || a->channels <= 0) {
    return AAC_ERROR_UNKNOWN_CHANNEL_CONFIGURATION;
  }

  if(AAC_NUM_SAMPLE_RATES <= a->ps_info_base.sampling_rate_index) {
    return AAC_ERROR_UNKNOWN_SAMPLING_FREQUENCY_INDEX;
  }

  a->tns_used = 0;
  a->pns_used = 0;

  s32 base_channel = 0;  
  
  do {

    // decode_next_element
    aac_unwrap(aac_decode_next_element(a, &b));

    if(a->curr_block_id < 0 ||
       AAC_ARRAY_LEN(AAC_ELEMENT_NUM_CHANNELS_TABLE) <= (u64) a->curr_block_id) {
      return AAC_ERROR_UNKNOWN_BLOCK_ID;
    }
    s32 element_channels = AAC_ELEMENT_NUM_CHANNELS_TABLE[a->curr_block_id];
    if(AAC_MAX_NCHANS < base_channel + element_channels) {
      return AAC_ERROR_CHANNELS_TOO_HIGH;
    }

    for(s32 channel=0;channel<element_channels;channel++) {
      // decode_noiseless_data
      aac_unwrap(aac_decode_noiseless_data(a, &b, channel));

      // dequantize
      aac_unwrap(aac_dequantize(a, channel));
    }

    if(a->curr_block_id == ___AAC_ID_CPE) {
      // stereo process
      aac_unwrap(aac_stereo_process(a));
    }

    for(s32 channel=0;channel<element_channels;channel++) {      
      // apply pns
      aac_unwrap(aac_apply_pns(a, channel));

      if(a->sbDeinterleaveReqd[channel] != 0) {	
	// deinterleave_short_blocks // why not deinterleave_channel ?
	aac_unwrap(aac_deinterleave_short_blocks(a, channel));
	
	a->sbDeinterleaveReqd[channel] = 0;
      }

      // apply tns
      aac_unwrap(aac_apply_tns(a, channel));
      
      // apply imdct
      if(output) aac_unwrap(aac_apply_imdct(a, channel, base_channel + channel, output));
    }
    
    base_channel = base_channel + element_channels;
  } while(a->curr_block_id != ___AAC_ID_END);

  if(!aac_bits_are_byte_aligned(b)) {
    // TODO: Verifiy this
    b.i = -1;
  }

  a->consumed = len - b.len;
  
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_ics(Aac_Ps_Info_Base *ps, Aac_Ics_Info *ics, Aac_Bits *b, s32 channel) {

  s32 global_gain;
  aac_unwrap(aac_bits_next_s32(b, 8, &global_gain));
  
  if(!ps->common_window) {
    aac_unwrap(aac_decode_ics_info(ics, b, ps->sampling_rate_index));
  }

  aac_unwrap(aac_decode_section_data(ps->sfb_code_book[channel],
				     b,				     
				     ics->window_seq,
				     ics->num_window_group,
				     ics->max_sfb));
  aac_unwrap(aac_decode_scale_factors(ps->scale_factors[channel],
				      b,
				      ps->sfb_code_book[channel],
				      ics->num_window_group,
				      ics->max_sfb,
				      global_gain));

  Aac_Pulse_Data *pulse = &ps->pulse_data[channel];
  aac_unwrap(aac_bits_next_u8(b, 1, &pulse->pulse_data_present));
  if(pulse->pulse_data_present) {
    aac_unwrap(aac_decode_pulse_data(pulse, b));
  }

  Aac_Tns_Data *tns = &ps->tns_data[channel];
  aac_unwrap(aac_bits_next_u8(b, 1, &tns->tns_data_present));
  if(tns->tns_data_present) {
    aac_unwrap(aac_decode_tns_data(tns, b, ics->window_seq));
  }

  Aac_Gain_Control_Data *gain = &ps->gain_control_data[channel];
  aac_unwrap(aac_bits_next_u8(b, 1, &gain->gain_control_data_present));
  if(gain->gain_control_data_present) {
    aac_unwrap(aac_decode_gain_control_data(gain, b, ics->window_seq));
  }
  
  return AAC_ERROR_NONE;
}

static s32 AAC_SFB_BAND_TAB_LONG_OFFSET[AAC_NUM_SAMPLE_RATES] = {0, 0, 42, 90, 90, 140, 192, 192, 240, 240, 240, 284};

static s16 AAC_SFB_BAND_TAB_LONG[325] = {
  /* long block 88, 96 kHz [42] (table 4.5.25) */
  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,   52,
  56,  64,  72,  80,  88,  96, 108, 120, 132, 144, 156, 172, 188,  212,
  240, 276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024,

  /* long block 64 kHz [48] (table 4.5.13) */
  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,   64,
  72,  80,  88, 100, 112, 124, 140, 156, 172, 192, 216, 240, 268, 304, 344,  384,
  424, 464, 504, 544, 584, 624, 664, 704, 744, 784, 824, 864, 904, 944, 984, 1024,

  /* long block 44, 48 kHz [50] (table 4.5.14) */
  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  48,  56,  64,  72,   80,  88,
  96, 108, 120, 132, 144, 160, 176, 196, 216, 240, 264, 292, 320, 352, 384,  416, 448,
  480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 1024,

  /* long block 32 kHz [52] (table 4.5.16) */
  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  48,  56,  64,  72,   80,  88,  96,
  108, 120, 132, 144, 160, 176, 196, 216, 240, 264, 292, 320, 352, 384, 416,  448, 480, 512,
  544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024,

  /* long block 22, 24 kHz [48] (table 4.5.21) */
  0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  52,  60,  68,   76,
  84,  92, 100, 108, 116, 124, 136, 148, 160, 172, 188, 204, 220, 240, 260,  284,
  308, 336, 364, 396, 432, 468, 508, 552, 600, 652, 704, 768, 832, 896, 960, 1024,

  /* long block 11, 12, 16 kHz [44] (table 4.5.19) */
  0,   8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88, 100,  112, 124,
  136, 148, 160, 172, 184, 196, 212, 228, 244, 260, 280, 300, 320,  344, 368,
  396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832, 896, 960, 1024,

  /* long block 8 kHz [41] (table 4.5.17) */
  0,  12,  24,  36,  48,  60,  72,  84,  96, 108, 120, 132,  144, 156,
  172, 188, 204, 220, 236, 252, 268, 288, 308, 328, 348, 372,  396, 420,
  448, 476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024
};

AAC_DEF Aac_Error aac_decode_spectrum_long(Aac_Ps_Info_Base *ps, Aac_Ics_Info *ics, Aac_Bits *b, s32 channel) {
  s32 *coefs = ps->coef[channel];

  s16 *sfb_tab = AAC_SFB_BAND_TAB_LONG + AAC_SFB_BAND_TAB_LONG_OFFSET[ps->sampling_rate_index];
  u8 *sfb_code_book = ps->sfb_code_book[channel];

  for(u8 sfb=0;sfb<ics->max_sfb;sfb++) {
    u8 cb = *sfb_code_book++;
    s32 value = sfb_tab[sfb + 1] - sfb_tab[sfb];

    if(cb == 0) {
      aac_coefs_unpack_zeros(coefs, value);
    } else if(cb <= 4) {
      aac_unwrap(aac_coefs_unpack_quads(coefs, b, cb, value));
    } else if(cb <= 10) {
      aac_unwrap(aac_coefs_unpack_pairs_no_esc(coefs, b, cb, value));
    } else if(cb == 11) {
      aac_unwrap(aac_coefs_unpack_pairs_esc(coefs, b, cb, value));
    } else {
      aac_coefs_unpack_zeros(coefs, value);
    }

    coefs += value;
  }
  
  s32 value = AAC_NSAMPS_LONG - sfb_tab[ics->max_sfb];
  aac_coefs_unpack_zeros(coefs, value);

  Aac_Pulse_Data *pulse = &ps->pulse_data[channel];
  if(pulse->pulse_data_present) {
    coefs = ps->coef[channel];
    s32 offset = (u8) sfb_tab[pulse->start_sfb];
    for(s32 i=0;i<pulse->num_pulse;i++) {
      offset += (s32) pulse->offset[i];
      if(coefs[offset] > 0) {
	coefs[offset] += pulse->amp[i];
      } else {
	coefs[offset] -= pulse->amp[i];
      }
      
    }
    
    if(AAC_NSAMPS_LONG <= (u64) offset) {
      return AAC_ERROR_UNREACHABLE;
    }
    
  }

  return AAC_ERROR_NONE;
}

/* scale factor band tables */
static s32 AAC_SFB_BAND_TAB_SHORT_OFFSET[AAC_NUM_SAMPLE_RATES] = {0, 0, 0, 13, 13, 13, 28, 28, 44, 44, 44, 60};

static s16 AAC_SFB_BAND_TAB_SHORT[76] = {
  /* short block 64, 88, 96 kHz [13] (tables 4.5.24, 4.5.26) */
  0,   4,   8,  12,  16,  20,  24,  32,  40,  48,  64,  92, 128,

  /* short block 32, 44, 48 kHz [15] (table 4.5.15) */
  0,   4,   8,  12,  16,  20,  28,  36,  44,  56,  68,  80,  96, 112, 128,

  /* short block 22, 24 kHz [16] (table 4.5.22) */
  0,   4,   8,  12,  16,  20,  24,  28,  36,  44,  52,  64,  76,  92, 108, 128,

  /* short block 11, 12, 16 kHz [16] (table 4.5.20) */
  0,   4,   8,  12,  16,  20,  24,  28,  32,  40,  48,  60,  72,  88, 108, 128,

  /* short block 8 kHz [16] (table 4.5.18) */
  0,   4,   8,  12,  16,  20,  24,  28,  36,  44,  52,  60,  72,  88, 108, 128
};

AAC_DEF Aac_Error aac_decode_spectrum_short(Aac_Ps_Info_Base *ps, Aac_Ics_Info *ics, Aac_Bits *b, s32 channel) {
  
  s16 *sfb_tab = AAC_SFB_BAND_TAB_SHORT + AAC_SFB_BAND_TAB_SHORT_OFFSET[ps->sampling_rate_index];
  u8 *sfb_code_book = ps->sfb_code_book[channel];

  s32 *coefs = ps->coef[channel];  
  for(u8 gp=0;gp<ics->num_window_group;gp++) {

    s32 value = 0;
    for(u8 sfb=0;sfb<ics->max_sfb;sfb++) {
      u8 cb = *sfb_code_book++;
      value = sfb_tab[sfb + 1] - sfb_tab[sfb];

      for(u8 win=0;win<ics->window_group_len[gp];win++) {
	s32 offset = win*AAC_NSAMPS_SHORT;

	if(cb == 0) {
	  aac_coefs_unpack_zeros(coefs + offset, value);
	} else if(cb <= 4) {
	  aac_unwrap(aac_coefs_unpack_quads(coefs + offset, b, cb, value));
	} else if(cb <= 10) {
	  aac_unwrap(aac_coefs_unpack_pairs_no_esc(coefs + offset, b, cb, value));
	} else if(cb == 11) {
	  aac_unwrap(aac_coefs_unpack_pairs_esc(coefs + offset, b, cb, value));
	} else {
	  aac_coefs_unpack_zeros(coefs + offset, value);
	}
      }
      coefs += value;
    }
    
    for(u8 win=0;win<ics->window_group_len[gp];win++) {
      s32 offset = win*AAC_NSAMPS_SHORT;
      value = AAC_NSAMPS_SHORT - sfb_tab[ics->max_sfb];
      aac_coefs_unpack_zeros(coefs + offset, value);
    }
    coefs += value;
    coefs += (ics->window_group_len[gp] - 1) * AAC_NSAMPS_SHORT;
  }

  if(coefs != (ps->coef[channel] + AAC_NSAMPS_LONG)) {
    return AAC_ERROR_UNREACHABLE;
  }

  return AAC_ERROR_NONE;    
}

AAC_DEF Aac_Error aac_decode_noiseless_data(Aac *a, Aac_Bits *b, s32 channel) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;

  Aac_Ics_Info *ics;
  if(channel == 1 && ps->common_window == 1) {
    ics = &ps->ics_info[0];
  } else {
    ics = &ps->ics_info[channel];
  }

  aac_unwrap(aac_decode_ics(ps, ics, b, channel)); 

  if(ics->window_seq == 2) {
    aac_unwrap(aac_decode_spectrum_short(ps, ics, b, channel));
  } else {
    aac_unwrap(aac_decode_spectrum_long(ps, ics, b, channel));
  }

  a->sbDeinterleaveReqd[channel] = 0;
  a->tns_used = a->tns_used | ps->tns_data[channel].tns_data_present;

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_dequantize(Aac *a, s32 channel) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;

  Aac_Ics_Info *ics;
  if(channel == 1 && ps->common_window == 1) {
    ics = &ps->ics_info[0];
  } else {
    ics = &ps->ics_info[channel];
  }

  s16 *sfb_tab;
  s32 number_of_samples;
  if(ics->window_seq == 2) {
    sfb_tab = AAC_SFB_BAND_TAB_SHORT + AAC_SFB_BAND_TAB_SHORT_OFFSET[ps->sampling_rate_index];
    number_of_samples = AAC_NSAMPS_SHORT;
    
  } else {
    sfb_tab = AAC_SFB_BAND_TAB_LONG + AAC_SFB_BAND_TAB_LONG_OFFSET[ps->sampling_rate_index];
    number_of_samples = AAC_NSAMPS_LONG;
    
  }

  s32 *coefs = ps->coef[channel];
  u8 *sfb_code_book = ps->sfb_code_book[channel];
  s16 *scale_factors = ps->scale_factors[channel];

  ps->intensity_used[channel] = 0;
  ps->pns_used[channel] = 0;

  s32 gb_mask = 0;

  for(u8 gp=0;gp<ics->num_window_group;gp++) {
    for(u8 win=0;win<ics->window_group_len[gp];win++) {
      for(u8 sfb=0;sfb<ics->max_sfb;sfb++) {
	
	s32 cb = (s32) sfb_code_book[sfb];
	s16 width = sfb_tab[sfb + 1] - sfb_tab[sfb];
	if(cb <= 11) { // 0 <= cb
	  gb_mask |= aac_coefs_dequantize_block(coefs, width, scale_factors[sfb]);
	} else if(cb == 13) {
	  ps->pns_used[channel] = 1;
	} else if(cb == 14 || cb == 15) {
	  ps->intensity_used[channel] = 1;
	}
	coefs += width;
	
      }      
      coefs += (number_of_samples - sfb_tab[ics->max_sfb]);
      
    }
    sfb_code_book += ics->max_sfb;
    scale_factors += ics->max_sfb;
    
  }
  a->pns_used |= ps->pns_used[channel];

  ps->gb_current[channel] = aac_clz(gb_mask) - 1;

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_stereo_process(Aac *a) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;

  if(ps->common_window != 1 || a->curr_block_id != ___AAC_ID_CPE) {
    return AAC_ERROR_NONE;
  }

  if(!ps->ms_mask_present && ps->intensity_used[1]) {
    return AAC_ERROR_NONE;
  }

  Aac_Ics_Info *ics = &ps->ics_info[0];
  s16 *sfb_tab;
  s32 number_of_samples;
  if(ics->window_seq == 2) {
    sfb_tab = AAC_SFB_BAND_TAB_SHORT + AAC_SFB_BAND_TAB_SHORT_OFFSET[ps->sampling_rate_index];
    number_of_samples = AAC_NSAMPS_SHORT;
    
  } else {
    sfb_tab = AAC_SFB_BAND_TAB_LONG + AAC_SFB_BAND_TAB_LONG_OFFSET[ps->sampling_rate_index];
    number_of_samples = AAC_NSAMPS_LONG;
    
  }

  s32 *coefs_left = ps->coef[0];
  s32 *coefs_right = ps->coef[1];

  s32 ms_mask_offset = 0;
  u8 *ms_mask_ptr = ps->ms_mask_bits;

  for(u8 gp=0;gp<ics->num_window_group;gp++) {
    for(u8 window=0;window<ics->window_group_len[gp];window++) {
      aac_coefs_stereo_process_group(coefs_left,
				     coefs_right,
				     sfb_tab,
				     ps->ms_mask_present,
				     ms_mask_ptr,
				     ms_mask_offset,
				     ics->max_sfb,
				     ps->sfb_code_book[1] + gp*ics->max_sfb,
				     ps->scale_factors[1] + gp*ics->max_sfb,
				     ps->gb_current);
      
      coefs_left += number_of_samples;
      coefs_right += number_of_samples;
    }
    ms_mask_ptr += (ms_mask_offset + ics->max_sfb) >> 3;
    ms_mask_offset = (ms_mask_offset + ics->max_sfb) & 0x07;
  }

  if((coefs_left != ps->coef[0] + 1024) ||
     (coefs_right != ps->coef[1] + 1024)) {
    return AAC_ERROR_UNREACHABLE;
  }
  
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_apply_pns(Aac *a, s32 channel) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;
  
  if(!ps->pns_used[channel]) {
    return AAC_ERROR_NONE;
  }

  Aac_Ics_Info *ics;
  if(channel == 1 && ps->common_window == 1) {
    ics = &ps->ics_info[0];
  } else {
    ics = &ps->ics_info[channel];
  }
  
  s16 *sfb_tab;
  s32 number_of_samples;
  if(ics->window_seq == 2) {
    sfb_tab = AAC_SFB_BAND_TAB_SHORT + AAC_SFB_BAND_TAB_SHORT_OFFSET[ps->sampling_rate_index];
    number_of_samples = AAC_NSAMPS_SHORT;
    
  } else {
    sfb_tab = AAC_SFB_BAND_TAB_LONG + AAC_SFB_BAND_TAB_LONG_OFFSET[ps->sampling_rate_index];
    number_of_samples = AAC_NSAMPS_LONG;
    
  }

  s32 *coefs = ps->coef[channel];
  u8 *sfb_code_book = ps->sfb_code_book[channel];
  s16 *scale_factors = ps->scale_factors[channel];

  int check_correction;
  if(a->curr_block_id == ___AAC_ID_CPE &&
     ps->common_window == 1) {
    check_correction = 1;
  } else {
    check_correction = 0;
  }

  s32 gb_mask = 0;
  for(u8 gp=0;gp<ics->num_window_group;gp++) {
    for(u8 win=0;win<ics->window_group_len[gp];win++) {
      u8 *ms_mask_ptr = ps->ms_mask_bits + ((gp*ics->max_sfb) >> 3);
      s32 ms_mask_offset = ((gp*ics->max_sfb) & 0x07);
      u8 ms_mask = (*ms_mask_ptr++) >> ms_mask_offset;

      for(u8 sfb=0;sfb<ics->max_sfb;sfb++) {
	s32 width = sfb_tab[sfb + 1] - sfb_tab[sfb];
	
	if(sfb_code_book[sfb] == 13) {
	  if(channel == 0) {
	    aac_coefs_generate_noise_vector(coefs,
					    &ps->pns_last_value,
					    width);

	    if(check_correction &&
	       ps->sfb_code_book[1][gp*ics->max_sfb + sfb] == 13) {
	      aac_coefs_copy_noise_vector(coefs,
					  ps->coef[1] + (coefs - ps->coef[0]),
					  width);
	    }
	    
	  } else {
	    s32 gen_new =
	      !(check_correction &&
	      ps->sfb_code_book[0][gp*ics->max_sfb + sfb] == 13 &&
	      (((ps->ms_mask_present == 1) && (ms_mask & 0x01)) ||
	       ps->ms_mask_present == 2
	       ));
	    
	    if(gen_new) {
	      aac_coefs_generate_noise_vector(coefs,
					      &ps->pns_last_value,
					      width);
	    }
	  }
	  
	  gb_mask |= aac_coefs_scale_noise_vector(coefs, width, ps->scale_factors[channel][gp*ics->max_sfb + sfb]);	  
	}
	coefs += width;

	ms_mask = ms_mask >> 1;
	if(++ms_mask_offset == 8) {
	  ms_mask = *ms_mask_ptr++;
	  ms_mask_offset = 0;
	}
      }
      
      coefs += (number_of_samples - sfb_tab[ics->max_sfb]);
    }
    
    sfb_code_book += ics->max_sfb;
    scale_factors += ics->max_sfb;
  }

  s32 gb = aac_clz(gb_mask) - 1;
  if(ps->gb_current[channel] > gb) {
    ps->gb_current[channel] = gb;
  }

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_deinterleave_short_blocks(Aac *a, s32 channel) {
  (void) a;
  (void) channel;
  AAC_TODO();
}

#define AAC_NWINDOWS_LONG 1
#define AAC_NWINDOWS_SHORT 8
#define ___AAC_NUM_SAMPLE_RATES 3

/* TNS max bands (table 4.139) and max order (table 4.138) */
static s32 AAC_TNS_MAX_BANDS_SHORT_OFFSET[___AAC_NUM_SAMPLE_RATES] = {0, 0, 12};

static u8 AAC_TNS_MAX_BANDS_SHORT[2*AAC_NUM_SAMPLE_RATES] = {
  /* short block, Main/LC */
  9,  9, 10, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  /* short block, SSR */
  7,  7,  7,  6,  6,  6,  7,  7,  8,  8,  8,  7,
};

static u8 AAC_TNS_MAX_ORDER_SHORT[___AAC_NUM_SAMPLE_RATES] = {7, 7, 7};

static s32 AAC_TNS_MAX_BANDS_LONG_OFFSET[___AAC_NUM_SAMPLE_RATES] = {0, 0, 12};

static u8 AAC_TNS_MAX_BANDS_LONG[2*AAC_NUM_SAMPLE_RATES] = {
  /* long block, Main/LC */
  31, 31, 34, 40, 42, 51, 46, 46, 42, 42, 42, 39,
  /* long block, SSR */
  28, 28, 27, 26, 26, 26, 29, 29, 23, 23, 23, 19,
};

static u8 AAC_TNS_MAX_ORDER_LONG[___AAC_NUM_SAMPLE_RATES] = {20, 12, 12};

static u8 AAC_SFB_BAND_TOTAL_SHORT[AAC_NUM_SAMPLE_RATES] = {
    12, 12, 12, 14, 14, 14, 15, 15, 15, 15, 15, 15
};

static u8 AAC_SFB_BAND_TOTAL_LONG[AAC_NUM_SAMPLE_RATES] = {
    41, 41, 47, 49, 49, 51, 47, 47, 43, 43, 43, 40
};

AAC_DEF Aac_Error aac_apply_tns(Aac *a, s32 channel) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;    
  Aac_Tns_Data *t = &ps->tns_data[channel];

  if(!t->tns_data_present) {
    return AAC_ERROR_NONE;
  }
  
  Aac_Ics_Info *ics;
  if(channel == 1 && ps->common_window == 1) {
    ics = &ps->ics_info[0];
  } else {
    ics = &ps->ics_info[channel];
  }

  s32 number_of_windows, window_len;
  u8 number_of_sfb;
  s32 max_order;
  s16 *sfb_tab;
  u8 *tns_max_band_tab;  
  if(ics->window_seq == 2) {
    number_of_windows = AAC_NWINDOWS_SHORT;
    window_len = AAC_NSAMPS_SHORT;
    number_of_sfb = AAC_SFB_BAND_TOTAL_SHORT[ps->sampling_rate_index];
    max_order = AAC_TNS_MAX_ORDER_SHORT[a->profile];
    sfb_tab = AAC_SFB_BAND_TAB_SHORT + AAC_SFB_BAND_TAB_SHORT_OFFSET[ps->sampling_rate_index];
    tns_max_band_tab = AAC_TNS_MAX_BANDS_SHORT + AAC_TNS_MAX_BANDS_SHORT_OFFSET[a->profile];
    
  } else {
    number_of_windows = AAC_NWINDOWS_LONG;
    window_len = AAC_NSAMPS_LONG;
    number_of_sfb = AAC_SFB_BAND_TOTAL_LONG[ps->sampling_rate_index];
    max_order = AAC_TNS_MAX_ORDER_LONG[a->profile];
    sfb_tab = AAC_SFB_BAND_TAB_LONG + AAC_SFB_BAND_TAB_LONG_OFFSET[ps->sampling_rate_index];
    tns_max_band_tab = AAC_TNS_MAX_BANDS_LONG + AAC_TNS_MAX_BANDS_LONG_OFFSET[a->profile];
    
  }

  s32 tns_max_band = tns_max_band_tab[ps->sampling_rate_index];

  if(tns_max_band > ics->max_sfb) {
    tns_max_band = ics->max_sfb;
  }

  u8 *filter_res = t->coef_res;
  u8 *filter_length = t->length;
  u8 *filter_order = t->order;
  u8 *filter_dir = t->dir;
  s8 *filter_coef = t->coef;

  s32 gb_mask = 0;
  s32 *audio_coefs = ps->coef[channel];

  for(s32 win=0;win<number_of_windows;win++) {
    s32 bottom = (s32) number_of_sfb;
    s32 num_filter = t->num_filt[win];
    for(s32 f=0;f<num_filter;f++) {
      s32 top = bottom;
      bottom = aac_max(top - *filter_length++, 0);      
      u8 order = aac_min(*filter_order++, max_order);

      if(order) {	
	s16 start = sfb_tab[aac_min(bottom, tns_max_band)];
	s16 end   = sfb_tab[aac_min(top, tns_max_band)];
	s16 size  = end - start;
					
	if(size > 0) {
	  u8 dir = *filter_dir++;
	  if(dir) {
	    start = end - 1;
	  }
	  
	  aac_tns_decode_lpc_coefs(ps->tns_lpc_buf,
				   ps->tns_work_buf,
				   filter_coef,
				   filter_res[win],
				   order);
	  
	  gb_mask |= aac_coefs_filter_region(audio_coefs + start,
					     ps->tns_lpc_buf,
					     ps->tns_work_buf,
					     size,
					     dir,
					     order);
	}
	filter_coef += order;
      }
    }
    audio_coefs += window_len;
  }

  s32 size = aac_clz(gb_mask) - 1;
  if(ps->gb_current[channel] > size) {
    ps->gb_current[channel] = size;

  }
  
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_apply_imdct(Aac *a, s32 channel, s32 out_channel, u8 *output) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;

  Aac_Ics_Info *ics;
  if(channel == 1 && ps->common_window == 1) {
    ics = &ps->ics_info[0];
  } else {
    ics = &ps->ics_info[channel];
  }
 
  output += (sizeof(short) * out_channel); // TODO: why ?
    
  if(ics->window_seq == 2) {
    for(s32 i=0;i<8;i++) {
      aac_coefs_dct4(ps->coef[channel] + i*128,
		     ps->gb_current[channel],
		     0);
    }
    
  } else {
    aac_coefs_dct4(ps->coef[channel],
		   ps->gb_current[channel],
		   1);
    
  }

  switch(ics->window_seq) {
  case 0: {
    aac_coefs_dec_window_overlap(ps->coef[channel],
				 ps->overlap[out_channel],
				 output,
				 a->channels,
				 ics->window_shape,
				 ps->prev_window_shape[out_channel]);
  } break;
  case 1: {
    aac_coefs_dec_window_overlap_long_start(ps->coef[channel],
					    ps->overlap[out_channel],
					    output,
					    a->channels,
					    ics->window_shape,
					    ps->prev_window_shape[out_channel]);
  } break;
  case 2: {
   aac_coefs_dec_window_overlap_short(ps->coef[channel],
				       ps->overlap[out_channel],
				       output,
				       a->channels,
				       ics->window_shape,
				       ps->prev_window_shape[out_channel]);
  } break;
  case 3: {
    aac_coefs_dec_window_overlap_long_stop(ps->coef[channel],
					   ps->overlap[out_channel],
					   output,
					   a->channels,
					   ics->window_shape,
					   ps->prev_window_shape[out_channel]);
  } break;
  }

  a->raw_sample_buf[channel] = 0;
  a->raw_sample_bytes = 0;
  a->raw_sample_fbits = 0;

  ps->prev_window_shape[out_channel] = ics->window_shape;
  
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_section_data(u8 *sfb_code_book,
					  Aac_Bits *b,
					  u8 window_seq,
					  u8 num_window_group,
					  u8 max_sfb) {

  s32 section_len_bits;
  if(window_seq == 2) {
    section_len_bits = 3;
  } else {
    section_len_bits = 5;
  }

  s32 section_escape_value = (1 << section_len_bits) - 1;

  for(s32 g=0;g<num_window_group;g++) {
    s32 sfb = 0;
    while(sfb < max_sfb) {
      s32 cb;
      aac_unwrap(aac_bits_next_s32(b, 4, &cb));
      
      s32 section_len = 0;
      s32 section_len_incr;
      do{
	aac_unwrap(aac_bits_next_s32(b, section_len_bits, &section_len_incr));
	section_len += section_len_incr;
	
      }while(section_len_incr == section_escape_value);

      sfb += section_len;
      while(section_len--) {
	*sfb_code_book++ = (u8) cb;
      }
      
    }

    if(sfb != max_sfb) {
      return AAC_ERROR_UNREACHABLE;
    }
  }

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_scale_factors(s16 *scale_factors,
					   Aac_Bits *b,
					   u8 *sfb_code_book,
					   u8 num_window_group,
					   u8 max_sfb,
					   s32 global_gain) {

  s32 sf = global_gain;
  s32 is = 0;
  s32 nrg = global_gain - 90 - 256;
  s32 npf = 1;

  for(s32 g=0;g<num_window_group * max_sfb;g++) {
    s32 sfb_cb = (s32) *sfb_code_book++;
    
    if(sfb_cb == 14 || sfb_cb == 15) {
      s32 value;
      aac_unwrap(aac_bits_decode_one_scale_factor(b, &value));
      is += value;
      *scale_factors++ = (s16) is;
      
    } else if(sfb_cb == 13) {
      s32 value;
      if(npf) {
        aac_unwrap(aac_bits_next_s32(b, 9, &value));
	npf = 0;
      } else {
	aac_unwrap(aac_bits_decode_one_scale_factor(b, &value));
      }
      nrg += value;
      *scale_factors++ = (s16) nrg;
      
    } else if(1 <= sfb_cb && sfb_cb <= 11) {
      s32 value;
      aac_unwrap(aac_bits_decode_one_scale_factor(b, &value));
      sf += value;
      *scale_factors++ = (s16) sf;
      
    } else {
      *scale_factors++ = (s16) 0;
      
    }
    
  }

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_pulse_data(Aac_Pulse_Data *p, Aac_Bits* b) {
  (void) p;
  (void) b;
  AAC_TODO();
}

static s8 AAC_SGN_MASK[3] = {0x02,  0x04,  0x08};
static s8 AAC_NEG_MASK[3] = {~0x03, ~0x07, ~0x0f};

AAC_DEF Aac_Error aac_decode_tns_data(Aac_Tns_Data *t, Aac_Bits* b, s32 window_seq) {

  s8 *coefs = t->coef;

  u8 *filter_length = t->length;
  u8 *filter_order = t->order;
  u8 *filter_dir = t->dir;

  if(window_seq == 2) {
    for(s32 w=0;w<AAC_NWINDOWS_SHORT;w++) {
      aac_unwrap(aac_bits_next_u8(b, 1, &t->num_filt[w]));
      if(!t->num_filt[w]) {
	continue;
      }

      aac_unwrap(aac_bits_next_u8(b, 1, &t->coef_res[w]));
      t->coef_res[w] += 3;
      aac_unwrap(aac_bits_next_u8(b, 4, filter_length));
      aac_unwrap(aac_bits_next_u8(b, 3, filter_order));
      if(*filter_order) {
	aac_unwrap(aac_bits_next_u8(b, 1, filter_dir++));
	s32 compress;
	aac_unwrap(aac_bits_next_s32(b, 1, &compress));
	s32 coef_bits = (s32) t->coef_res[w] - compress;
	
	s8 s = AAC_SGN_MASK[coef_bits - 2];
	s8 n = AAC_NEG_MASK[coef_bits - 2];
	for(u8 i=0;i<(*filter_order);i++) {
	  s8 c;
	  aac_unwrap(aac_bits_next_s8(b, coef_bits, &c));
	  if(c & s) {
	    c = c | n;
	  }
	  *coefs++ = c;
	}
      }
      filter_length++;
      filter_order++;
    }
    
  } else {
    aac_unwrap(aac_bits_next_u8(b, 2, &t->num_filt[0]));
    if(t->num_filt[0]) {
      aac_unwrap(aac_bits_next_u8(b, 1, &t->coef_res[0]));
      t->coef_res[0] += 3;
    }

    for(u8 f=0;f<t->num_filt[0];f++) {
      aac_unwrap(aac_bits_next_u8(b, 6, filter_length));
      aac_unwrap(aac_bits_next_u8(b, 5, filter_order));
      if(*filter_order) {
	aac_unwrap(aac_bits_next_u8(b, 1, filter_dir++));
	s32 compress;
	aac_unwrap(aac_bits_next_s32(b, 1, &compress));
	s32 coef_bits = (s32) t->coef_res[0] - compress;

	s8 s = AAC_SGN_MASK[coef_bits - 2];
	s8 n = AAC_NEG_MASK[coef_bits - 2];
	for(u8 i=0;i<(*filter_order);i++) {
	  s8 c;
	  aac_unwrap(aac_bits_next_s8(b, coef_bits, &c));
	  if(c & s) {
	    c = c | n;
	  }
	  *coefs++ = c;
	}
	
      }
      filter_length++;
      filter_order++;
      
    }
    
  }

  return AAC_ERROR_NONE;
}

static u8 AAC_GAIN_BITS[4][3] = {
  {1, 5, 5},  /* long */
  {2, 4, 2},  /* start */
  {8, 2, 2},  /* short */
  {2, 4, 5},  /* stop */
};

AAC_DEF Aac_Error aac_decode_gain_control_data(Aac_Gain_Control_Data *g, Aac_Bits *b, u8 window_seq) {
  
  aac_unwrap(aac_bits_next_u8(b, 2, &g->max_band));
  
  u8 max_window       = AAC_GAIN_BITS[window_seq][0];
  u8 locale_bits_zero = AAC_GAIN_BITS[window_seq][1];
  u8 locale_bits      = AAC_GAIN_BITS[window_seq][2];

  for(u8 bd=1;bd<=g->max_band;bd++) {
    for(u8 wd=0;wd<max_window;wd++) {
      aac_unwrap(aac_bits_next_u8(b, 3, &g->adj_num[bd][wd]));

      for(u8 ad=0;ad<g->adj_num[bd][wd];ad++) {
	aac_unwrap(aac_bits_next_u8(b, 4, &g->alev_code[bd][wd][ad]));

        u8 number_of_bits;
	if(wd == 0) {
	  number_of_bits = locale_bits_zero;
	} else {
	  number_of_bits = locale_bits;
	}

	aac_unwrap(aac_bits_next_u8(b, number_of_bits, &g->aloc_code[bd][wd][ad]));
      }
    }
  }
  
  return AAC_ERROR_NONE;
}

static s32 AAC_INV_QUANT_3[16] = {
	0x00000000, 0xc8767f65, 0x9becf22c, 0x83358feb, 0x83358feb, 0x9becf22c, 0xc8767f65, 0x00000000,
	0x2bc750e9, 0x5246dd49, 0x6ed9eba1, 0x7e0e2e32, 0x7e0e2e32, 0x6ed9eba1, 0x5246dd49, 0x2bc750e9,
};

static s32 AAC_INV_QUANT_4[16] = {
	0x00000000, 0xe5632654, 0xcbf00dbe, 0xb4c373ee, 0xa0e0a15f, 0x9126145f, 0x8643c7b3, 0x80b381ac,
	0x7f7437ad, 0x7b1d1a49, 0x7294b5f2, 0x66256db2, 0x563ba8aa, 0x4362210e, 0x2e3d2abb, 0x17851aad,
};

#define AAC_FBITS_LPC_COEFS 20

AAC_DEF void aac_tns_decode_lpc_coefs(s32 *tns_lpc_buf,
				      s32 *tns_work_buf,
				      s8 *filter_coef,
				      u8 filter_res,
				      u8 order) {

  s32 *inv_quant_tab;
  if(filter_res == 3) {
    inv_quant_tab = AAC_INV_QUANT_3;
  } else if(filter_res == 4) {
    inv_quant_tab = AAC_INV_QUANT_4;
  } else {
    return;
  }

  for(u8 m=0;m<order;m++) {
    s32 t = inv_quant_tab[filter_coef[m] & 0x0f];
    for(u8 i=0;i<m;i++) {
      tns_work_buf[i] =
	tns_lpc_buf[i] - (aac_mulshift_32(t, tns_lpc_buf[m - i - 1]) << 1);
    }
    for(u8 i=0;i<m;i++) {
      tns_lpc_buf[i] = tns_work_buf[i];
    }
    tns_lpc_buf[m] = t >> (31 - AAC_FBITS_LPC_COEFS);
  }
  
}

AAC_DEF void aac_coefs_unpack_zeros(s32 *coefs, s32 value) {

  // TODO: check that value is multiple of 4 ?

  while(value > 0) {
    *coefs++ = 0;
    *coefs++ = 0;
    *coefs++ = 0;
    *coefs++ = 0;
    value -= 4;
  }
  
}

static Aac_Huff_Info AAC_HUFF_TAB_SPEC_INFO[11] = {
  /* table 0 not used */
  {11, {  1,  0,  0,  0,  8,  0, 24,  0, 24,  8, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0},   0},
  { 9, {  0,  0,  1,  1,  7, 24, 15, 19, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},  81},
  {16, {  1,  0,  0,  4,  2,  6,  3,  5, 15, 15,  8,  9,  3,  3,  5,  2,  0,  0,  0,  0}, 162},
  {12, {  0,  0,  0, 10,  6,  0,  9, 21,  8, 14, 11,  2,  0,  0,  0,  0,  0,  0,  0,  0}, 243},
  {13, {  1,  0,  0,  4,  4,  0,  4, 12, 12, 12, 18, 10,  4,  0,  0,  0,  0,  0,  0,  0}, 324},
  {11, {  0,  0,  0,  9,  0, 16, 13,  8, 23,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 405},
  {12, {  1,  0,  2,  1,  0,  4,  5, 10, 14, 15,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0}, 486},
  {10, {  0,  0,  1,  5,  7, 10, 14, 15,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 550},
  {15, {  1,  0,  2,  1,  0,  4,  3,  8, 11, 20, 31, 38, 32, 14,  4,  0,  0,  0,  0,  0}, 614},
  {12, {  0,  0,  0,  3,  8, 14, 17, 25, 31, 41, 22,  8,  0,  0,  0,  0,  0,  0,  0,  0}, 783},
  {12, {  0,  0,  0,  2,  6,  7, 16, 59, 55, 95, 43,  6,  0,  0,  0,  0,  0,  0,  0,  0}, 952},
};

#define AAC_HUFF_TAB_SPEC_OFFSET 1

static s16 AAC_HUFF_TAB_SPEC[1241] = {
  /* spectrum table 1 [81] (signed) */
  0x0000, 0x0200, 0x0e00, 0x0007, 0x0040, 0x0001, 0x0038, 0x0008, 0x01c0, 0x03c0, 0x0e40, 0x0039, 0x0078, 0x01c8, 0x000f, 0x0240, 
  0x003f, 0x0fc0, 0x01f8, 0x0238, 0x0047, 0x0e08, 0x0009, 0x0208, 0x01c1, 0x0048, 0x0041, 0x0e38, 0x0201, 0x0e07, 0x0207, 0x0e01, 
  0x01c7, 0x0278, 0x0e78, 0x03c8, 0x004f, 0x0079, 0x01c9, 0x01cf, 0x03f8, 0x0239, 0x007f, 0x0e48, 0x0e0f, 0x0fc8, 0x01f9, 0x03c1, 
  0x03c7, 0x0e47, 0x0ff8, 0x01ff, 0x0049, 0x020f, 0x0241, 0x0e41, 0x0248, 0x0fc1, 0x0e3f, 0x0247, 0x023f, 0x0e39, 0x0fc7, 0x0e09, 
  0x0209, 0x03cf, 0x0e79, 0x0e4f, 0x03f9, 0x0249, 0x0fc9, 0x027f, 0x0fcf, 0x0fff, 0x0279, 0x03c9, 0x0e49, 0x0e7f, 0x0ff9, 0x03ff, 
  0x024f, 
  /* spectrum table 2 [81] (signed) */
  0x0000, 0x0200, 0x0e00, 0x0001, 0x0038, 0x0007, 0x01c0, 0x0008, 0x0040, 0x01c8, 0x0e40, 0x0078, 0x000f, 0x0047, 0x0039, 0x0e07, 
  0x03c0, 0x0238, 0x0fc0, 0x003f, 0x0208, 0x0201, 0x01c1, 0x0e08, 0x0041, 0x01f8, 0x0e01, 0x01c7, 0x0e38, 0x0240, 0x0048, 0x0009, 
  0x0207, 0x0079, 0x0239, 0x0e78, 0x01cf, 0x03c8, 0x0247, 0x0209, 0x0e48, 0x01f9, 0x0248, 0x0e0f, 0x0ff8, 0x0e39, 0x03f8, 0x0278, 
  0x03c1, 0x0e47, 0x0fc8, 0x0e09, 0x0fc1, 0x0fc7, 0x01ff, 0x020f, 0x023f, 0x007f, 0x0049, 0x0e41, 0x0e3f, 0x004f, 0x03c7, 0x01c9, 
  0x0241, 0x03cf, 0x0e79, 0x03f9, 0x0fff, 0x0e4f, 0x0e49, 0x0249, 0x0fcf, 0x03c9, 0x0e7f, 0x0fc9, 0x027f, 0x03ff, 0x0ff9, 0x0279, 
  0x024f, 
  /* spectrum table 3 [81] (unsigned) */
  0x0000, 0x1200, 0x1001, 0x1040, 0x1008, 0x2240, 0x2009, 0x2048, 0x2041, 0x2208, 0x3049, 0x2201, 0x3248, 0x4249, 0x3209, 0x3241, 
  0x1400, 0x1002, 0x200a, 0x2440, 0x3288, 0x2011, 0x3051, 0x2280, 0x304a, 0x3448, 0x1010, 0x2088, 0x2050, 0x1080, 0x2042, 0x2408, 
  0x4289, 0x3089, 0x3250, 0x4251, 0x3281, 0x2210, 0x3211, 0x2081, 0x4449, 0x424a, 0x3441, 0x320a, 0x2012, 0x3052, 0x3488, 0x3290, 
  0x2202, 0x2401, 0x3091, 0x2480, 0x4291, 0x3242, 0x3409, 0x4252, 0x4489, 0x2090, 0x308a, 0x3212, 0x3481, 0x3450, 0x3490, 0x3092, 
  0x4491, 0x4451, 0x428a, 0x4292, 0x2082, 0x2410, 0x3282, 0x3411, 0x444a, 0x3442, 0x4492, 0x448a, 0x4452, 0x340a, 0x2402, 0x3482, 
  0x3412, 
  /* spectrum table 4 [81] (unsigned) */
  0x4249, 0x3049, 0x3241, 0x3248, 0x3209, 0x1200, 0x2240, 0x0000, 0x2009, 0x2208, 0x2201, 0x2048, 0x1001, 0x2041, 0x1008, 0x1040, 
  0x4449, 0x4251, 0x4289, 0x424a, 0x3448, 0x3441, 0x3288, 0x3409, 0x3051, 0x304a, 0x3250, 0x3089, 0x320a, 0x3281, 0x3242, 0x3211, 
  0x2440, 0x2408, 0x2280, 0x2401, 0x2042, 0x2088, 0x200a, 0x2050, 0x2081, 0x2202, 0x2011, 0x2210, 0x1400, 0x1002, 0x1080, 0x1010, 
  0x4291, 0x4489, 0x4451, 0x4252, 0x428a, 0x444a, 0x3290, 0x3488, 0x3450, 0x3091, 0x3052, 0x3481, 0x308a, 0x3411, 0x3212, 0x4491, 
  0x3282, 0x340a, 0x3442, 0x4292, 0x4452, 0x448a, 0x2090, 0x2480, 0x2012, 0x2410, 0x2082, 0x2402, 0x4492, 0x3092, 0x3490, 0x3482, 
  0x3412, 
  /* spectrum table 5 [81] (signed) */
  0x0000, 0x03e0, 0x0020, 0x0001, 0x001f, 0x003f, 0x03e1, 0x03ff, 0x0021, 0x03c0, 0x0002, 0x0040, 0x001e, 0x03df, 0x0041, 0x03fe, 
  0x0022, 0x03c1, 0x005f, 0x03e2, 0x003e, 0x03a0, 0x0060, 0x001d, 0x0003, 0x03bf, 0x0023, 0x0061, 0x03fd, 0x03a1, 0x007f, 0x003d, 
  0x03e3, 0x03c2, 0x0042, 0x03de, 0x005e, 0x03be, 0x007e, 0x03c3, 0x005d, 0x0062, 0x0043, 0x03a2, 0x03dd, 0x001c, 0x0380, 0x0081, 
  0x0080, 0x039f, 0x0004, 0x009f, 0x03fc, 0x0024, 0x03e4, 0x0381, 0x003c, 0x007d, 0x03bd, 0x03a3, 0x03c4, 0x039e, 0x0082, 0x005c, 
  0x0044, 0x0063, 0x0382, 0x03dc, 0x009e, 0x007c, 0x039d, 0x0383, 0x0064, 0x03a4, 0x0083, 0x009d, 0x03bc, 0x009c, 0x0384, 0x0084, 
  0x039c, 
  /* spectrum table 6 [81] (signed) */
  0x0000, 0x0020, 0x001f, 0x0001, 0x03e0, 0x0021, 0x03e1, 0x003f, 0x03ff, 0x005f, 0x0041, 0x03c1, 0x03df, 0x03c0, 0x03e2, 0x0040, 
  0x003e, 0x0022, 0x001e, 0x03fe, 0x0002, 0x005e, 0x03c2, 0x03de, 0x0042, 0x03a1, 0x0061, 0x007f, 0x03e3, 0x03bf, 0x0023, 0x003d, 
  0x03fd, 0x0060, 0x03a0, 0x001d, 0x0003, 0x0062, 0x03be, 0x03c3, 0x0043, 0x007e, 0x005d, 0x03dd, 0x03a2, 0x0063, 0x007d, 0x03bd, 
  0x03a3, 0x003c, 0x03fc, 0x0081, 0x0381, 0x039f, 0x0024, 0x009f, 0x03e4, 0x001c, 0x0382, 0x039e, 0x0044, 0x03dc, 0x0380, 0x0082, 
  0x009e, 0x03c4, 0x0080, 0x005c, 0x0004, 0x03bc, 0x03a4, 0x007c, 0x009d, 0x0064, 0x0083, 0x0383, 0x039d, 0x0084, 0x0384, 0x039c, 
  0x009c, 
  /* spectrum table 7 [64] (unsigned) */
  0x0000, 0x0420, 0x0401, 0x0821, 0x0841, 0x0822, 0x0440, 0x0402, 0x0861, 0x0823, 0x0842, 0x0460, 0x0403, 0x0843, 0x0862, 0x0824, 
  0x0881, 0x0825, 0x08a1, 0x0863, 0x0844, 0x0404, 0x0480, 0x0882, 0x0845, 0x08a2, 0x0405, 0x08c1, 0x04a0, 0x0826, 0x0883, 0x0865, 
  0x0864, 0x08a3, 0x0846, 0x08c2, 0x0827, 0x0866, 0x0406, 0x04c0, 0x0884, 0x08e1, 0x0885, 0x08e2, 0x08a4, 0x08c3, 0x0847, 0x08e3, 
  0x08c4, 0x08a5, 0x0886, 0x0867, 0x04e0, 0x0407, 0x08c5, 0x08a6, 0x08e4, 0x0887, 0x08a7, 0x08e5, 0x08e6, 0x08c6, 0x08c7, 0x08e7, 
  /* spectrum table 8 [64] (unsigned) */
  0x0821, 0x0841, 0x0420, 0x0822, 0x0401, 0x0842, 0x0000, 0x0440, 0x0402, 0x0861, 0x0823, 0x0862, 0x0843, 0x0863, 0x0881, 0x0824, 
  0x0882, 0x0844, 0x0460, 0x0403, 0x0883, 0x0864, 0x08a2, 0x08a1, 0x0845, 0x0825, 0x08a3, 0x0865, 0x0884, 0x08a4, 0x0404, 0x0885, 
  0x0480, 0x0846, 0x08c2, 0x08c1, 0x0826, 0x0866, 0x08c3, 0x08a5, 0x04a0, 0x08c4, 0x0405, 0x0886, 0x08e1, 0x08e2, 0x0847, 0x08c5, 
  0x08e3, 0x0827, 0x08a6, 0x0867, 0x08c6, 0x08e4, 0x04c0, 0x0887, 0x0406, 0x08e5, 0x08e6, 0x08c7, 0x08a7, 0x04e0, 0x0407, 0x08e7, 	
  /* spectrum table 9 [169] (unsigned) */
  0x0000, 0x0420, 0x0401, 0x0821, 0x0841, 0x0822, 0x0440, 0x0402, 0x0861, 0x0842, 0x0823, 0x0460, 0x0403, 0x0843, 0x0862, 0x0824, 
  0x0881, 0x0844, 0x0825, 0x0882, 0x0863, 0x0404, 0x0480, 0x08a1, 0x0845, 0x0826, 0x0864, 0x08a2, 0x08c1, 0x0883, 0x0405, 0x0846, 
  0x04a0, 0x0827, 0x0865, 0x0828, 0x0901, 0x0884, 0x08a3, 0x08c2, 0x08e1, 0x0406, 0x0902, 0x0848, 0x0866, 0x0847, 0x0885, 0x0921, 
  0x0829, 0x08e2, 0x04c0, 0x08a4, 0x08c3, 0x0903, 0x0407, 0x0922, 0x0868, 0x0886, 0x0867, 0x0408, 0x0941, 0x08c4, 0x0849, 0x08a5, 
  0x0500, 0x04e0, 0x08e3, 0x0942, 0x0923, 0x0904, 0x082a, 0x08e4, 0x08c5, 0x08a6, 0x0888, 0x0887, 0x0869, 0x0961, 0x08a8, 0x0520, 
  0x0905, 0x0943, 0x084a, 0x0409, 0x0962, 0x0924, 0x08c6, 0x0981, 0x0889, 0x0906, 0x082b, 0x0925, 0x0944, 0x08a7, 0x08e5, 0x084b, 
  0x082c, 0x0982, 0x0963, 0x086a, 0x08a9, 0x08c7, 0x0907, 0x0964, 0x040a, 0x08e6, 0x0983, 0x0540, 0x0945, 0x088a, 0x08c8, 0x084c, 
  0x0926, 0x0927, 0x088b, 0x0560, 0x08c9, 0x086b, 0x08aa, 0x0908, 0x08e8, 0x0985, 0x086c, 0x0965, 0x08e7, 0x0984, 0x0966, 0x0946, 
  0x088c, 0x08e9, 0x08ab, 0x040b, 0x0986, 0x08ca, 0x0580, 0x0947, 0x08ac, 0x08ea, 0x0928, 0x040c, 0x0967, 0x0909, 0x0929, 0x0948, 
  0x08eb, 0x0987, 0x08cb, 0x090b, 0x0968, 0x08ec, 0x08cc, 0x090a, 0x0949, 0x090c, 0x092a, 0x092b, 0x092c, 0x094b, 0x0989, 0x094a, 
  0x0969, 0x0988, 0x096a, 0x098a, 0x098b, 0x094c, 0x096b, 0x096c, 0x098c, 
  /* spectrum table 10 [169] (unsigned) */
  0x0821, 0x0822, 0x0841, 0x0842, 0x0420, 0x0401, 0x0823, 0x0862, 0x0861, 0x0843, 0x0863, 0x0440, 0x0402, 0x0844, 0x0882, 0x0824, 
  0x0881, 0x0000, 0x0883, 0x0864, 0x0460, 0x0403, 0x0884, 0x0845, 0x08a2, 0x0825, 0x08a1, 0x08a3, 0x0865, 0x08a4, 0x0885, 0x08c2, 
  0x0846, 0x08c3, 0x0480, 0x08c1, 0x0404, 0x0826, 0x0866, 0x08a5, 0x08c4, 0x0886, 0x08c5, 0x08e2, 0x0867, 0x0847, 0x08a6, 0x0902, 
  0x08e3, 0x04a0, 0x08e1, 0x0405, 0x0901, 0x0827, 0x0903, 0x08e4, 0x0887, 0x0848, 0x08c6, 0x08e5, 0x0828, 0x0868, 0x0904, 0x0888, 
  0x08a7, 0x0905, 0x08a8, 0x08e6, 0x08c7, 0x0922, 0x04c0, 0x08c8, 0x0923, 0x0869, 0x0921, 0x0849, 0x0406, 0x0906, 0x0924, 0x0889, 
  0x0942, 0x0829, 0x08e7, 0x0907, 0x0925, 0x08e8, 0x0943, 0x08a9, 0x0944, 0x084a, 0x0941, 0x086a, 0x0926, 0x08c9, 0x0500, 0x088a, 
  0x04e0, 0x0962, 0x08e9, 0x0963, 0x0946, 0x082a, 0x0961, 0x0927, 0x0407, 0x0908, 0x0945, 0x086b, 0x08aa, 0x0909, 0x0965, 0x0408, 
  0x0964, 0x084b, 0x08ea, 0x08ca, 0x0947, 0x088b, 0x082b, 0x0982, 0x0928, 0x0983, 0x0966, 0x08ab, 0x0984, 0x0967, 0x0985, 0x086c, 
  0x08cb, 0x0520, 0x0948, 0x0540, 0x0981, 0x0409, 0x088c, 0x0929, 0x0986, 0x084c, 0x090a, 0x092a, 0x082c, 0x0968, 0x0987, 0x08eb, 
  0x08ac, 0x08cc, 0x0949, 0x090b, 0x0988, 0x040a, 0x08ec, 0x0560, 0x094a, 0x0969, 0x096a, 0x040b, 0x096b, 0x092b, 0x094b, 0x0580, 
  0x090c, 0x0989, 0x094c, 0x092c, 0x096c, 0x098b, 0x040c, 0x098a, 0x098c, 
  /* spectrum table 11 [289] (unsigned) */
  0x0000, 0x2041, 0x2410, 0x1040, 0x1001, 0x2081, 0x2042, 0x2082, 0x2043, 0x20c1, 0x20c2, 0x1080, 0x2083, 0x1002, 0x20c3, 0x2101, 
  0x2044, 0x2102, 0x2084, 0x2103, 0x20c4, 0x10c0, 0x1003, 0x2141, 0x2142, 0x2085, 0x2104, 0x2045, 0x2143, 0x20c5, 0x2144, 0x2105, 
  0x2182, 0x2086, 0x2181, 0x2183, 0x20c6, 0x2046, 0x2110, 0x20d0, 0x2405, 0x2403, 0x2404, 0x2184, 0x2406, 0x1100, 0x2106, 0x1004, 
  0x2090, 0x2145, 0x2150, 0x2407, 0x2402, 0x2408, 0x2087, 0x21c2, 0x20c7, 0x2185, 0x2146, 0x2190, 0x240a, 0x21c3, 0x21c1, 0x2409, 
  0x21d0, 0x2050, 0x2047, 0x2107, 0x240b, 0x21c4, 0x240c, 0x2210, 0x2401, 0x2186, 0x2250, 0x2088, 0x2147, 0x2290, 0x240d, 0x2203, 
  0x2202, 0x20c8, 0x1140, 0x240e, 0x22d0, 0x21c5, 0x2108, 0x2187, 0x21c6, 0x1005, 0x2204, 0x240f, 0x2310, 0x2048, 0x2201, 0x2390, 
  0x2148, 0x2350, 0x20c9, 0x2205, 0x21c7, 0x2089, 0x2206, 0x2242, 0x2243, 0x23d0, 0x2109, 0x2188, 0x1180, 0x2244, 0x2149, 0x2207, 
  0x21c8, 0x2049, 0x2283, 0x1006, 0x2282, 0x2241, 0x2245, 0x210a, 0x208a, 0x2246, 0x20ca, 0x2189, 0x2284, 0x2208, 0x2285, 0x2247, 
  0x22c3, 0x204a, 0x11c0, 0x2286, 0x21c9, 0x20cb, 0x214a, 0x2281, 0x210b, 0x22c2, 0x2342, 0x218a, 0x2343, 0x208b, 0x1400, 0x214b, 
  0x22c5, 0x22c4, 0x2248, 0x21ca, 0x2209, 0x1010, 0x210d, 0x1007, 0x20cd, 0x22c6, 0x2341, 0x2344, 0x2303, 0x208d, 0x2345, 0x220a, 
  0x218b, 0x2288, 0x2287, 0x2382, 0x2304, 0x204b, 0x210c, 0x22c1, 0x20cc, 0x204d, 0x2302, 0x21cb, 0x20ce, 0x214c, 0x214d, 0x2384, 
  0x210e, 0x22c7, 0x2383, 0x2305, 0x2346, 0x2306, 0x1200, 0x22c8, 0x208c, 0x2249, 0x2385, 0x218d, 0x228a, 0x23c2, 0x220b, 0x224a, 
  0x2386, 0x2289, 0x214e, 0x22c9, 0x2381, 0x208e, 0x218c, 0x204c, 0x2348, 0x1008, 0x2347, 0x21cc, 0x2307, 0x21cd, 0x23c3, 0x2301, 
  0x218e, 0x208f, 0x23c5, 0x23c4, 0x204e, 0x224b, 0x210f, 0x2387, 0x220d, 0x2349, 0x220c, 0x214f, 0x20cf, 0x228b, 0x22ca, 0x2308, 
  0x23c6, 0x23c7, 0x220e, 0x23c1, 0x21ce, 0x1240, 0x1009, 0x224d, 0x224c, 0x2309, 0x2388, 0x228d, 0x2389, 0x230a, 0x218f, 0x21cf, 
  0x224e, 0x23c8, 0x22cb, 0x22ce, 0x204f, 0x228c, 0x228e, 0x234b, 0x234a, 0x22cd, 0x22cc, 0x220f, 0x238b, 0x234c, 0x230d, 0x23c9, 
  0x238a, 0x1280, 0x230b, 0x224f, 0x100a, 0x230c, 0x12c0, 0x230e, 0x228f, 0x234d, 0x100d, 0x238c, 0x23ca, 0x23cb, 0x22cf, 0x238d, 
  0x1340, 0x100b, 0x234e, 0x23cc, 0x23cd, 0x230f, 0x1380, 0x238e, 0x234f, 0x1300, 0x238f, 0x100e, 0x100c, 0x23ce, 0x13c0, 0x100f, 
  0x23cf, 
};

#define aac_huffman_scalar_get_quad_w(v) (((s32) (v) << 20) >> 29)
#define aac_huffman_scalar_get_quad_x(v) (((s32) (v) << 23) >> 29)
#define aac_huffman_scalar_get_quad_y(v) (((s32) (v) << 26) >> 29)
#define aac_huffman_scalar_get_quad_z(v) (((s32) (v) << 29) >> 29)
#define aac_huffman_scalar_get_quad_sign_bits(v) (((u32) (v) << 17) >> 29)
#define aac_huffman_scalar_apply_sign(v, s) {(v) ^= ((s32) (s) >> 31); (v) -= ((s32) (s) >> 31);}
#define aac_huffman_scalar_get_pair_y(v) (((s32) (v) << 22) >> 27)
#define aac_huffman_scalar_get_pair_z(v) (((s32) (v) << 27) >> 27)
#define aac_huffman_scalar_get_pair_sign_bits(v) (((u32) (v) << 20) >> 30)
#define aac_huffman_scalar_get_pair_esc_y(v) (((s32) (v) << 20) >> 26)
#define aac_huffman_scalar_get_pair_esc_z(v) (((s32) (v) << 26) >> 26)
#define aac_huffman_scalar_get_pair_esc_sign_bits(v) (((u32) (v) << 18) >> 30)


AAC_DEF Aac_Error aac_coefs_unpack_quads(s32 *coefs, Aac_Bits *b, u8 cb, s32 value) {
  s32 max_bits = AAC_HUFF_TAB_SPEC_INFO[cb - AAC_HUFF_TAB_SPEC_OFFSET].max_bits + 4;

  while(value > 0) {
    u32 bit_buf;
    aac_unwrap(aac_bits_peek_u32(b, max_bits, &bit_buf));
    bit_buf = bit_buf << (32 - max_bits);

    s32 scalar;
    s32 n_code_bits = aac_decode_huffman_scalar(AAC_HUFF_TAB_SPEC,
						&AAC_HUFF_TAB_SPEC_INFO[cb - AAC_HUFF_TAB_SPEC_OFFSET],
					       bit_buf,
					       &scalar);

    s32 w = aac_huffman_scalar_get_quad_w(scalar);
    s32 x = aac_huffman_scalar_get_quad_x(scalar);
    s32 y = aac_huffman_scalar_get_quad_y(scalar);
    s32 z = aac_huffman_scalar_get_quad_z(scalar);
    s32 sign_bits = (s32) aac_huffman_scalar_get_quad_sign_bits(scalar);
    
    bit_buf = bit_buf << n_code_bits;
    aac_unwrap(aac_bits_discard(b, (u64) (n_code_bits + sign_bits)));

    if(sign_bits) {
      if(w) {
	aac_huffman_scalar_apply_sign(w, bit_buf);
	bit_buf = bit_buf << 1;
      }
      if(x) {
	aac_huffman_scalar_apply_sign(x, bit_buf);
	bit_buf = bit_buf << 1;
      }
      if(y) {
	aac_huffman_scalar_apply_sign(y, bit_buf);
	bit_buf = bit_buf << 1;
      }
      if(z) {
	aac_huffman_scalar_apply_sign(z, bit_buf);
	bit_buf = bit_buf << 1;
      }
      
    }
    
    *coefs++ = w;
    *coefs++ = x;
    *coefs++ = y;
    *coefs++ = z;
    
    value -= 4;
  }

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_coefs_unpack_pairs_no_esc(s32 *coefs, Aac_Bits *b, u8 cb, s32 value) {

  s32 max_bits = AAC_HUFF_TAB_SPEC_INFO[cb - AAC_HUFF_TAB_SPEC_OFFSET].max_bits + 2;

  while(value > 0) {
    u32 bit_buf;
    aac_unwrap(aac_bits_peek_u32(b, max_bits, &bit_buf));
    bit_buf = bit_buf << (32 - max_bits);

    s32 scalar;
    s32 n_code_bits = aac_decode_huffman_scalar(AAC_HUFF_TAB_SPEC,
						&AAC_HUFF_TAB_SPEC_INFO[cb - AAC_HUFF_TAB_SPEC_OFFSET],
						bit_buf,
						&scalar);

    s32 y = aac_huffman_scalar_get_pair_y(scalar);
    s32 z = aac_huffman_scalar_get_pair_z(scalar);    
    s32 sign_bits = aac_huffman_scalar_get_pair_sign_bits(scalar);

    bit_buf = bit_buf << n_code_bits;
    aac_unwrap(aac_bits_discard(b, (u64) (n_code_bits + sign_bits)));
    if(sign_bits) {
      if(y) {
	aac_huffman_scalar_apply_sign(y, bit_buf);
	bit_buf = bit_buf << 1;	
      }
      if(z) {
	aac_huffman_scalar_apply_sign(z, bit_buf);
	bit_buf = bit_buf << 1;
      }
    }

    *coefs++ = y;
    *coefs++ = z;
    value -= 2;
  }

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_coefs_unpack_pairs_esc(s32 *coefs, Aac_Bits *b, u8 cb, s32 value) {
  
  s32 max_bits = AAC_HUFF_TAB_SPEC_INFO[cb - AAC_HUFF_TAB_SPEC_OFFSET].max_bits + 2;

  while(value > 0) {
    u32 bit_buf;
    aac_unwrap(aac_bits_peek_u32(b, max_bits, &bit_buf));
    bit_buf = bit_buf << (32 - max_bits);

    s32 scalar;
    s32 n_code_bits = aac_decode_huffman_scalar(AAC_HUFF_TAB_SPEC,
						&AAC_HUFF_TAB_SPEC_INFO[cb - AAC_HUFF_TAB_SPEC_OFFSET],
						bit_buf,
						&scalar);

    s32 y = aac_huffman_scalar_get_pair_esc_y(scalar);
    s32 z = aac_huffman_scalar_get_pair_esc_z(scalar);    
    s32 sign_bits = aac_huffman_scalar_get_pair_esc_sign_bits(scalar);

    bit_buf = bit_buf << n_code_bits;
    aac_unwrap(aac_bits_discard(b, (u64) (n_code_bits + sign_bits)));

    if(y == 16) {
      s32 n = 4;
      
      while(1) {
	u8 bit;
	aac_unwrap(aac_bits_next_u8(b, 1, &bit));
	if(bit != 1) break;
	
	n++;
      }

      s32 m;
      aac_unwrap(aac_bits_next_s32(b, n, &m));
      y = (1 << n) + m;
    }
    
    if(z == 16) {
      s32 n = 4;
      
      while(1) {
	u8 bit;
	aac_unwrap(aac_bits_next_u8(b, 1, &bit));
	if(bit != 1) break;
	
	n++;
      }

      s32 m;
      aac_unwrap(aac_bits_next_s32(b, n, &m));
      z = (1 << n) + m;
      
    }
    
    if(sign_bits) {
      if(y) {
	aac_huffman_scalar_apply_sign(y, bit_buf);
	bit_buf = bit_buf << 1;	
      }
      if(z) {
	aac_huffman_scalar_apply_sign(z, bit_buf);
	bit_buf = bit_buf << 1;
      }
    }

    *coefs++ = y;
    *coefs++ = z;
    value -= 2;
  }

  return AAC_ERROR_NONE;
}

static s32 AAC_POW_43_14[4][16] = {
  {
    0x00000000, 0x10000000, 0x285145f3, 0x453a5cdb, /* Q28 */
    0x0cb2ff53, 0x111989d6, 0x15ce31c8, 0x1ac7f203, /* Q25 */
    0x20000000, 0x257106b9, 0x2b16b4a3, 0x30ed74b4, /* Q25 */
    0x36f23fa5, 0x3d227bd3, 0x437be656, 0x49fc823c, /* Q25 */
  },
  {
    0x00000000, 0x1306fe0a, 0x2ff221af, 0x52538f52, 
    0x0f1a1bf4, 0x1455ccc2, 0x19ee62a8, 0x1fd92396, 
    0x260dfc14, 0x2c8694d8, 0x333dcb29, 0x3a2f5c7a, 
    0x4157aed5, 0x48b3aaa3, 0x50409f76, 0x57fc3010, 
  },
  {
    0x00000000, 0x16a09e66, 0x39047c0f, 0x61e734aa, 
    0x11f59ac4, 0x182ec633, 0x1ed66a45, 0x25dfc55a, 
    0x2d413ccd, 0x34f3462d, 0x3cefc603, 0x4531ab69, 
    0x4db4adf8, 0x56752054, 0x5f6fcfcd, 0x68a1eca1, 
  },
  {
    0x00000000, 0x1ae89f99, 0x43ce3e4b, 0x746d57b2, 
    0x155b8109, 0x1cc21cdc, 0x24ac1839, 0x2d0a479e, 
    0x35d13f33, 0x3ef80748, 0x48775c93, 0x524938cd, 
    0x5c68841d, 0x66d0df0a, 0x717e7bfe, 0x7c6e0305, 
  },
};

static s32 AAC_POW_14[4] = {
  0x40000000, 0x4c1bf829, 0x5a82799a, 0x6ba27e65
};

static s32 AAC_POLY_43_LO[5] = {
  0x29a0bda9, 0xb02e4828, 0x5957aa1b, 0x236c498d, 0xff581859
};

static s32 AAC_POLY_43_HI[5] = {
  0x10852163, 0xd333f6a4, 0x46e9408b, 0x27c2cef0, 0xfef577b4
};

static s32 AAC_POW_2_FRAC[8] = {
	0x6597fa94, 0x50a28be6, 0x7fffffff, 0x6597fa94, 
	0x50a28be6, 0x7fffffff, 0x6597fa94, 0x50a28be6
};

static s32 AAC_POW_2_EXP[8] = {
  14, 13, 11, 10, 9, 7, 6, 5
};

static s32 AAC_POW_43[48] = {
  0x1428a2fa, 0x15db1bd6, 0x1796302c, 0x19598d85, 
  0x1b24e8bb, 0x1cf7fcfa, 0x1ed28af2, 0x20b4582a, 
  0x229d2e6e, 0x248cdb55, 0x26832fda, 0x28800000, 
  0x2a832287, 0x2c8c70a8, 0x2e9bc5d8, 0x30b0ff99, 
  0x32cbfd4a, 0x34eca001, 0x3712ca62, 0x393e6088, 
  0x3b6f47e0, 0x3da56717, 0x3fe0a5fc, 0x4220ed72, 
  0x44662758, 0x46b03e7c, 0x48ff1e87, 0x4b52b3f3, 
  0x4daaebfd, 0x5007b497, 0x5268fc62, 0x54ceb29c, 
  0x5738c721, 0x59a72a59, 0x5c19cd35, 0x5e90a129, 
  0x610b9821, 0x638aa47f, 0x660db90f, 0x6894c90b, 
  0x6b1fc80c, 0x6daeaa0d, 0x70416360, 0x72d7e8b0, 
  0x75722ef9, 0x78102b85, 0x7ab1d3ec, 0x7d571e09, 
};

AAC_DEF s32 aac_coefs_dequantize_block(s32 *coefs, s16 width, s16 scale) {
  if(width <= 0) {
    return 0;
  }

  scale -= AAC_SF_OFFSET;

  s32 *tab_16 = AAC_POW_43_14[scale & 0x3];
  s32 scale_f = AAC_POW_14[scale & 0x3];
  s32 scale_i = (scale >> 2) + AAC_FBITS_OUT_DQ_OFF;

  s32 tab_4[4];
  s32 shift = 28 - scale_i;
  if(shift > 31) {
    tab_4[0] = 0;
    tab_4[1] = 0;
    tab_4[2] = 0;
    tab_4[3] = 0;
  } else if(shift <= 0) {
    shift = -shift;
    if(shift > 31) {
      shift = 31;
    }

    for(s32 x=0;x<4;x++) {
      s32 y = tab_16[x];
      if(y > (0x7fffffff >> shift)) {
	y = 0x7fffffff;
      } else {
	y = y << shift;
      }
      
      tab_4[x] = y;
    }
        
  } else {
    tab_4[0] = 0;
    tab_4[1] = tab_16[1] >> shift;
    tab_4[2] = tab_16[2] >> shift;
    tab_4[3] = tab_16[3] >> shift;
    
  }

  s32 gb_mask = 0;
  do {

    s32 coef = *coefs;
    s32 x = aac_fastabs(coef);

    s32 y;
    if(x < 4) {
      y = tab_4[x];
    } else {
      if(x < 16) {
	y = tab_16[x];
	shift = 25 - scale_i;
	
      } else if(x < 64) {
        y = AAC_POW_43[x - 16];
	shift = 21 - scale_i;
	y = aac_mulshift_32(y, scale_f);
	
      } else {
	x = x << 17;
	shift = 0;
	if(x < 0x08000000) {
	  x = x << 4;
	  shift = shift + 4;
	}
	if(x < 0x20000000) {
	  x = x << 2;
	  shift = shift + 2;
	}
	if(x < 0x40000000) {
	  x = x << 1;
	  shift = shift + 1;
	}

	s32 *new_coefs;
	if(x < AAC_SQRTHALF) {
	  new_coefs = AAC_POLY_43_LO;
	} else {
	  new_coefs = AAC_POLY_43_HI;
	}

	y = new_coefs[0];
	y = aac_mulshift_32(y, x) + new_coefs[1];
	y = aac_mulshift_32(y, x) + new_coefs[2];
	y = aac_mulshift_32(y, x) + new_coefs[3];
	y = aac_mulshift_32(y, x) + new_coefs[4];
	y = aac_mulshift_32(y, AAC_POW_2_FRAC[shift]) << 3;

	y = aac_mulshift_32(y, scale_f);
	shift = 24 - scale_i - AAC_POW_2_EXP[shift];
      }

      if(shift <= 0) {
	shift = -shift;
	if(shift > 31) {
	  shift = 31;
	}

	if(y > (0x7fffffff >> shift)) {
	  y = 0x7fffffff;
	} else {
	  y = y << shift;
	}
	
      } else {
	if(shift > 31) {
	  shift = 31;
	}
	y = y >> shift;
	
      }
    }    
    gb_mask |= y;    

    coef = coef >> 31;
    y = y ^ coef;
    y = y - coef;

    *coefs++ = y;

  } while(--width);

  return gb_mask;
}

static s32 AAC_POW_14_STEREO_PROCESS[2][4] = {
  { 0xc0000000, 0xb3e407d7, 0xa57d8666, 0x945d819b }, 
  { 0x40000000, 0x4c1bf829, 0x5a82799a, 0x6ba27e65 }
};

AAC_DEF void aac_coefs_stereo_process_group(s32 *coefs_left,
					    s32 *coefs_right,
					    s16 *sfb_tab,
					    s32 ms_mask_present,
					    u8 *ms_mask_ptr,
					    s32 ms_mask_offset,
					    u8 max_sfb,
					    u8 *sfb_code_book,
					    s16 *scale_factors,
					    s32* gb_current) {

  u8 ms_mask = (*ms_mask_ptr++) >> ms_mask_offset;
  s32 gb_mask_left = 0;
  s32 gb_mask_right = 0;

  for(u8 sfb=0;sfb<max_sfb;sfb++) {
    s32 width = sfb_tab[sfb + 1] - sfb_tab[sfb];
    u8 cb_index = sfb_code_book[sfb];

    if(cb_index == 14 || cb_index == 15) {
      
      if(ms_mask_present == 1 && (ms_mask & 0x01)) {
	cb_index ^= 0x01;
      }

      s32 sf = - scale_factors[sfb];

      cb_index = cb_index & 0x01;
      s32 scale_f = AAC_POW_14_STEREO_PROCESS[cb_index][sf & 0x03];
      s32 scale_i = (sf >> 2) + 2;

      if(scale_i > 0) {
	if(scale_i > 30) {
	  scale_i = 30;
	}

	do{
	  s32 cr = aac_mulshift_32(*coefs_left++, scale_f);
	  aac_clip_2n(cr, 31 - scale_i);
	  cr = cr << scale_i;
	  gb_mask_right |= aac_fastabs(cr);
	  *coefs_right++ = cr;
	  
	} while(--width);
	
      } else {
	scale_i = -scale_i;
	if(scale_i > 31) {
	  scale_i = 31;
	}

	do {
	  s32 cr = aac_mulshift_32(*coefs_left++, scale_f) >> scale_i;
	  gb_mask_right |= aac_fastabs(cr);
	  *coefs_right++ = cr;
	  
	} while(--width);
	
      }
      
    } else if(cb_index != 13 &&
	      (ms_mask_present == 2 || ((ms_mask_present == 1) && (ms_mask & 0x01)) )
	      ) {

      do {
	s32 cl = *coefs_left;
	s32 cr = *coefs_right;

	s32 sf;
	if( (aac_fastabs(cl) | aac_fastabs(cr) ) >> 30 ) {
	  cl = cl >> 1;
	  
	  sf = cl + (cr >> 1);
	  aac_clip_2n(sf, 30);
	  sf = sf << 1;
	  
	  cl = cl - (cr >> 1);
	  aac_clip_2n(cl, 30);
	  cl = cl << 1;
	  
	} else {
	  sf = cl + cr;
	  cl -= cr;
	  
	}

	*coefs_left++ = sf;
	gb_mask_left |= aac_fastabs(sf);

	*coefs_right++= cl;
	gb_mask_right |= aac_fastabs(cl);
	
      } while(--width);
      
    } else {
      
      coefs_left  += width;
      coefs_right += width;
    }

    ms_mask = ms_mask >> 1;
    if(++ms_mask_offset == 8) {
      ms_mask = *ms_mask_ptr++;
      ms_mask_offset = 0;
    }
  }

  s32 cl = aac_clz(gb_mask_left) - 1;
  if(gb_current[0] > cl) {
    gb_current[0] = cl;
  }

  s32 cr = aac_clz(gb_mask_right) - 1;
  if(gb_current[1] > cr) {
    gb_current[1] = cr;
  }
  
}

AAC_DEF void aac_coefs_dct4(s32 *coefs, s32 gb_current, s32 tab) {
  
  if(gb_current < AAC_GBITS_IN_DCT4) {
    s32 es = AAC_GBITS_IN_DCT4 - gb_current;
    
    aac_coefs_pre_multiply_rescale(coefs, es, tab);
    aac_coefs_r4fft(coefs, tab);
    aac_coefs_post_multiply_rescale(coefs, es, tab);
    
  } else {
    aac_coefs_pre_multiply(coefs, tab);
    aac_coefs_r4fft(coefs, tab);
    aac_coefs_post_multiply(coefs, tab);
    
  }
  
}

static s32 AAC_KBD_WINDOW_OFFSET[AAC_NUM_IMDCT_SIZES] = {0, 128};

static s32 AAC_KBD_WINDOW[128 + 1024] = {
  /* 128 - format = Q31 * 2^0 */
  0x00016f63, 0x7ffffffe, 0x0003e382, 0x7ffffff1, 0x00078f64, 0x7fffffc7, 0x000cc323, 0x7fffff5d, 
  0x0013d9ed, 0x7ffffe76, 0x001d3a9d, 0x7ffffcaa, 0x0029581f, 0x7ffff953, 0x0038b1bd, 0x7ffff372, 
  0x004bd34d, 0x7fffe98b, 0x00635538, 0x7fffd975, 0x007fdc64, 0x7fffc024, 0x00a219f1, 0x7fff995b, 
  0x00cacad0, 0x7fff5f5b, 0x00fab72d, 0x7fff0a75, 0x0132b1af, 0x7ffe9091, 0x01739689, 0x7ffde49e, 
  0x01be4a63, 0x7ffcf5ef, 0x0213b910, 0x7ffbaf84, 0x0274d41e, 0x7ff9f73a, 0x02e2913a, 0x7ff7acf1, 
  0x035de86c, 0x7ff4a99a, 0x03e7d233, 0x7ff0be3d, 0x0481457c, 0x7febb2f1, 0x052b357c, 0x7fe545d4, 
  0x05e68f77, 0x7fdd2a02, 0x06b4386f, 0x7fd30695, 0x07950acb, 0x7fc675b4, 0x0889d3ef, 0x7fb703be, 
  0x099351e0, 0x7fa42e89, 0x0ab230e0, 0x7f8d64d8, 0x0be70923, 0x7f7205f8, 0x0d325c93, 0x7f516195, 
  0x0e9494ae, 0x7f2ab7d0, 0x100e0085, 0x7efd3997, 0x119ed2ef, 0x7ec8094a, 0x134720d8, 0x7e8a3ba7, 
  0x1506dfdc, 0x7e42d906, 0x16dde50b, 0x7df0dee4, 0x18cbe3f7, 0x7d9341b4, 0x1ad06e07, 0x7d28ef02, 
  0x1ceaf215, 0x7cb0cfcc, 0x1f1abc4f, 0x7c29cb20, 0x215ef677, 0x7b92c8eb, 0x23b6a867, 0x7aeab4ec, 
  0x2620b8ec, 0x7a3081d0, 0x289beef5, 0x79632c5a, 0x2b26f30b, 0x7881be95, 0x2dc0511f, 0x778b5304, 
  0x30667aa2, 0x767f17c0, 0x3317c8dd, 0x755c5178, 0x35d27f98, 0x74225e50, 0x3894cff3, 0x72d0b887, 
  0x3b5cdb7b, 0x7166f8e7, 0x3e28b770, 0x6fe4d8e8, 0x40f6702a, 0x6e4a3491, 0x43c40caa, 0x6c970bfc, 
  0x468f9231, 0x6acb8483, 0x495707f5, 0x68e7e994, 0x4c187ac7, 0x66ecad1c, 0x4ed200c5, 0x64da6797, 
  0x5181bcea, 0x62b1d7b7, 0x5425e28e, 0x6073e1ae, 0x56bcb8c2, 0x5e218e16, 0x59449d76, 0x5bbc0875, 
  /* 1024 - format = Q31 * 2^0 */
  0x0009962f, 0x7fffffa4, 0x000e16fb, 0x7fffff39, 0x0011ea65, 0x7ffffebf, 0x0015750e, 0x7ffffe34, 
  0x0018dc74, 0x7ffffd96, 0x001c332e, 0x7ffffce5, 0x001f83f5, 0x7ffffc1f, 0x0022d59a, 0x7ffffb43, 
  0x00262cc2, 0x7ffffa4f, 0x00298cc4, 0x7ffff942, 0x002cf81f, 0x7ffff81a, 0x003070c4, 0x7ffff6d6, 
  0x0033f840, 0x7ffff573, 0x00378fd9, 0x7ffff3f1, 0x003b38a1, 0x7ffff24d, 0x003ef381, 0x7ffff085, 
  0x0042c147, 0x7fffee98, 0x0046a2a8, 0x7fffec83, 0x004a9847, 0x7fffea44, 0x004ea2b7, 0x7fffe7d8, 
  0x0052c283, 0x7fffe53f, 0x0056f829, 0x7fffe274, 0x005b4422, 0x7fffdf76, 0x005fa6dd, 0x7fffdc43, 
  0x006420c8, 0x7fffd8d6, 0x0068b249, 0x7fffd52f, 0x006d5bc4, 0x7fffd149, 0x00721d9a, 0x7fffcd22, 
  0x0076f828, 0x7fffc8b6, 0x007bebca, 0x7fffc404, 0x0080f8d9, 0x7fffbf06, 0x00861fae, 0x7fffb9bb, 
  0x008b609e, 0x7fffb41e, 0x0090bbff, 0x7fffae2c, 0x00963224, 0x7fffa7e1, 0x009bc362, 0x7fffa13a, 
  0x00a17009, 0x7fff9a32, 0x00a7386c, 0x7fff92c5, 0x00ad1cdc, 0x7fff8af0, 0x00b31da8, 0x7fff82ad, 
  0x00b93b21, 0x7fff79f9, 0x00bf7596, 0x7fff70cf, 0x00c5cd57, 0x7fff672a, 0x00cc42b1, 0x7fff5d05, 
  0x00d2d5f3, 0x7fff525c, 0x00d9876c, 0x7fff4729, 0x00e05769, 0x7fff3b66, 0x00e74638, 0x7fff2f10, 
  0x00ee5426, 0x7fff221f, 0x00f58182, 0x7fff148e, 0x00fcce97, 0x7fff0658, 0x01043bb3, 0x7ffef776, 
  0x010bc923, 0x7ffee7e2, 0x01137733, 0x7ffed795, 0x011b4631, 0x7ffec68a, 0x01233669, 0x7ffeb4ba, 
  0x012b4827, 0x7ffea21d, 0x01337bb8, 0x7ffe8eac, 0x013bd167, 0x7ffe7a61, 0x01444982, 0x7ffe6533, 
  0x014ce454, 0x7ffe4f1c, 0x0155a229, 0x7ffe3813, 0x015e834d, 0x7ffe2011, 0x0167880c, 0x7ffe070d, 
  0x0170b0b2, 0x7ffdecff, 0x0179fd8b, 0x7ffdd1df, 0x01836ee1, 0x7ffdb5a2, 0x018d0500, 0x7ffd9842, 
  0x0196c035, 0x7ffd79b3, 0x01a0a0ca, 0x7ffd59ee, 0x01aaa70a, 0x7ffd38e8, 0x01b4d341, 0x7ffd1697, 
  0x01bf25b9, 0x7ffcf2f2, 0x01c99ebd, 0x7ffccdee, 0x01d43e99, 0x7ffca780, 0x01df0597, 0x7ffc7f9e, 
  0x01e9f401, 0x7ffc563d, 0x01f50a22, 0x7ffc2b51, 0x02004844, 0x7ffbfecf, 0x020baeb1, 0x7ffbd0ab, 
  0x02173db4, 0x7ffba0da, 0x0222f596, 0x7ffb6f4f, 0x022ed6a1, 0x7ffb3bfd, 0x023ae11f, 0x7ffb06d8, 
  0x02471558, 0x7ffacfd3, 0x02537397, 0x7ffa96e0, 0x025ffc25, 0x7ffa5bf2, 0x026caf4a, 0x7ffa1efc, 
  0x02798d4f, 0x7ff9dfee, 0x0286967c, 0x7ff99ebb, 0x0293cb1b, 0x7ff95b55, 0x02a12b72, 0x7ff915ab, 
  0x02aeb7cb, 0x7ff8cdaf, 0x02bc706d, 0x7ff88351, 0x02ca559f, 0x7ff83682, 0x02d867a9, 0x7ff7e731, 
  0x02e6a6d2, 0x7ff7954e, 0x02f51361, 0x7ff740c8, 0x0303ad9c, 0x7ff6e98e, 0x031275ca, 0x7ff68f8f, 
  0x03216c30, 0x7ff632ba, 0x03309116, 0x7ff5d2fb, 0x033fe4bf, 0x7ff57042, 0x034f6773, 0x7ff50a7a, 
  0x035f1975, 0x7ff4a192, 0x036efb0a, 0x7ff43576, 0x037f0c78, 0x7ff3c612, 0x038f4e02, 0x7ff35353, 
  0x039fbfeb, 0x7ff2dd24, 0x03b06279, 0x7ff26370, 0x03c135ed, 0x7ff1e623, 0x03d23a8b, 0x7ff16527, 
  0x03e37095, 0x7ff0e067, 0x03f4d84e, 0x7ff057cc, 0x040671f7, 0x7fefcb40, 0x04183dd3, 0x7fef3aad, 
  0x042a3c22, 0x7feea5fa, 0x043c6d25, 0x7fee0d11, 0x044ed11d, 0x7fed6fda, 0x04616849, 0x7fecce3d, 
  0x047432eb, 0x7fec2821, 0x04873140, 0x7feb7d6c, 0x049a6388, 0x7feace07, 0x04adca01, 0x7fea19d6, 
  0x04c164ea, 0x7fe960c0, 0x04d53481, 0x7fe8a2aa, 0x04e93902, 0x7fe7df79, 0x04fd72aa, 0x7fe71712, 
  0x0511e1b6, 0x7fe6495a, 0x05268663, 0x7fe57634, 0x053b60eb, 0x7fe49d83, 0x05507189, 0x7fe3bf2b, 
  0x0565b879, 0x7fe2db0f, 0x057b35f4, 0x7fe1f110, 0x0590ea35, 0x7fe10111, 0x05a6d574, 0x7fe00af3, 
  0x05bcf7ea, 0x7fdf0e97, 0x05d351cf, 0x7fde0bdd, 0x05e9e35c, 0x7fdd02a6, 0x0600acc8, 0x7fdbf2d2, 
  0x0617ae48, 0x7fdadc40, 0x062ee814, 0x7fd9becf, 0x06465a62, 0x7fd89a5e, 0x065e0565, 0x7fd76eca, 
  0x0675e954, 0x7fd63bf1, 0x068e0662, 0x7fd501b0, 0x06a65cc3, 0x7fd3bfe4, 0x06beecaa, 0x7fd2766a, 
  0x06d7b648, 0x7fd1251e, 0x06f0b9d1, 0x7fcfcbda, 0x0709f775, 0x7fce6a7a, 0x07236f65, 0x7fcd00d8, 
  0x073d21d2, 0x7fcb8ecf, 0x07570eea, 0x7fca1439, 0x077136dd, 0x7fc890ed, 0x078b99da, 0x7fc704c7, 
  0x07a6380d, 0x7fc56f9d, 0x07c111a4, 0x7fc3d147, 0x07dc26cc, 0x7fc2299e, 0x07f777b1, 0x7fc07878, 
  0x0813047d, 0x7fbebdac, 0x082ecd5b, 0x7fbcf90f, 0x084ad276, 0x7fbb2a78, 0x086713f7, 0x7fb951bc, 
  0x08839206, 0x7fb76eaf, 0x08a04ccb, 0x7fb58126, 0x08bd446e, 0x7fb388f4, 0x08da7915, 0x7fb185ee, 
  0x08f7eae7, 0x7faf77e5, 0x09159a09, 0x7fad5ead, 0x0933869f, 0x7fab3a17, 0x0951b0cd, 0x7fa909f6, 
  0x097018b7, 0x7fa6ce1a, 0x098ebe7f, 0x7fa48653, 0x09ada248, 0x7fa23273, 0x09ccc431, 0x7f9fd249, 
  0x09ec245b, 0x7f9d65a4, 0x0a0bc2e7, 0x7f9aec53, 0x0a2b9ff3, 0x7f986625, 0x0a4bbb9e, 0x7f95d2e7, 
  0x0a6c1604, 0x7f933267, 0x0a8caf43, 0x7f908472, 0x0aad8776, 0x7f8dc8d5, 0x0ace9eb9, 0x7f8aff5c, 
  0x0aeff526, 0x7f8827d3, 0x0b118ad8, 0x7f854204, 0x0b335fe6, 0x7f824dbb, 0x0b557469, 0x7f7f4ac3, 
  0x0b77c879, 0x7f7c38e4, 0x0b9a5c2b, 0x7f7917e9, 0x0bbd2f97, 0x7f75e79b, 0x0be042d0, 0x7f72a7c3, 
  0x0c0395ec, 0x7f6f5828, 0x0c2728fd, 0x7f6bf892, 0x0c4afc16, 0x7f6888c9, 0x0c6f0f4a, 0x7f650894, 
  0x0c9362a8, 0x7f6177b9, 0x0cb7f642, 0x7f5dd5ff, 0x0cdcca26, 0x7f5a232a, 0x0d01de63, 0x7f565f00, 
  0x0d273307, 0x7f528947, 0x0d4cc81f, 0x7f4ea1c2, 0x0d729db7, 0x7f4aa835, 0x0d98b3da, 0x7f469c65, 
  0x0dbf0a92, 0x7f427e13, 0x0de5a1e9, 0x7f3e4d04, 0x0e0c79e7, 0x7f3a08f9, 0x0e339295, 0x7f35b1b4, 
  0x0e5aebfa, 0x7f3146f8, 0x0e82861a, 0x7f2cc884, 0x0eaa60fd, 0x7f28361b, 0x0ed27ca5, 0x7f238f7c, 
  0x0efad917, 0x7f1ed467, 0x0f237656, 0x7f1a049d, 0x0f4c5462, 0x7f151fdc, 0x0f75733d, 0x7f1025e3, 
  0x0f9ed2e6, 0x7f0b1672, 0x0fc8735e, 0x7f05f146, 0x0ff254a1, 0x7f00b61d, 0x101c76ae, 0x7efb64b4, 
  0x1046d981, 0x7ef5fcca, 0x10717d15, 0x7ef07e19, 0x109c6165, 0x7eeae860, 0x10c7866a, 0x7ee53b5b, 
  0x10f2ec1e, 0x7edf76c4, 0x111e9279, 0x7ed99a58, 0x114a7971, 0x7ed3a5d1, 0x1176a0fc, 0x7ecd98eb, 
  0x11a30910, 0x7ec77360, 0x11cfb1a1, 0x7ec134eb, 0x11fc9aa2, 0x7ebadd44, 0x1229c406, 0x7eb46c27, 
  0x12572dbf, 0x7eade14c, 0x1284d7bc, 0x7ea73c6c, 0x12b2c1ed, 0x7ea07d41, 0x12e0ec42, 0x7e99a382, 
  0x130f56a8, 0x7e92aee7, 0x133e010b, 0x7e8b9f2a, 0x136ceb59, 0x7e847402, 0x139c157b, 0x7e7d2d25, 
  0x13cb7f5d, 0x7e75ca4c, 0x13fb28e6, 0x7e6e4b2d, 0x142b1200, 0x7e66af7f, 0x145b3a92, 0x7e5ef6f8, 
  0x148ba281, 0x7e572150, 0x14bc49b4, 0x7e4f2e3b, 0x14ed300f, 0x7e471d70, 0x151e5575, 0x7e3eeea5, 
  0x154fb9c9, 0x7e36a18e, 0x15815ced, 0x7e2e35e2, 0x15b33ec1, 0x7e25ab56, 0x15e55f25, 0x7e1d019e, 
  0x1617bdf9, 0x7e14386e, 0x164a5b19, 0x7e0b4f7d, 0x167d3662, 0x7e02467e, 0x16b04fb2, 0x7df91d25, 
  0x16e3a6e2, 0x7defd327, 0x17173bce, 0x7de66837, 0x174b0e4d, 0x7ddcdc0a, 0x177f1e39, 0x7dd32e53, 
  0x17b36b69, 0x7dc95ec6, 0x17e7f5b3, 0x7dbf6d17, 0x181cbcec, 0x7db558f9, 0x1851c0e9, 0x7dab221f, 
  0x1887017d, 0x7da0c83c, 0x18bc7e7c, 0x7d964b05, 0x18f237b6, 0x7d8baa2b, 0x19282cfd, 0x7d80e563, 
  0x195e5e20, 0x7d75fc5e, 0x1994caee, 0x7d6aeed0, 0x19cb7335, 0x7d5fbc6d, 0x1a0256c2, 0x7d5464e6, 
  0x1a397561, 0x7d48e7ef, 0x1a70cede, 0x7d3d453b, 0x1aa86301, 0x7d317c7c, 0x1ae03195, 0x7d258d65, 
  0x1b183a63, 0x7d1977aa, 0x1b507d30, 0x7d0d3afc, 0x1b88f9c5, 0x7d00d710, 0x1bc1afe6, 0x7cf44b97, 
  0x1bfa9f58, 0x7ce79846, 0x1c33c7e0, 0x7cdabcce, 0x1c6d293f, 0x7ccdb8e4, 0x1ca6c337, 0x7cc08c39, 
  0x1ce0958a, 0x7cb33682, 0x1d1a9ff8, 0x7ca5b772, 0x1d54e240, 0x7c980ebd, 0x1d8f5c21, 0x7c8a3c14, 
  0x1dca0d56, 0x7c7c3f2e, 0x1e04f59f, 0x7c6e17bc, 0x1e4014b4, 0x7c5fc573, 0x1e7b6a53, 0x7c514807, 
  0x1eb6f633, 0x7c429f2c, 0x1ef2b80f, 0x7c33ca96, 0x1f2eaf9e, 0x7c24c9fa, 0x1f6adc98, 0x7c159d0d, 
  0x1fa73eb2, 0x7c064383, 0x1fe3d5a3, 0x7bf6bd11, 0x2020a11e, 0x7be7096c, 0x205da0d8, 0x7bd7284a, 
  0x209ad483, 0x7bc71960, 0x20d83bd1, 0x7bb6dc65, 0x2115d674, 0x7ba6710d, 0x2153a41b, 0x7b95d710, 
  0x2191a476, 0x7b850e24, 0x21cfd734, 0x7b7415ff, 0x220e3c02, 0x7b62ee59, 0x224cd28d, 0x7b5196e9, 
  0x228b9a82, 0x7b400f67, 0x22ca938a, 0x7b2e578a, 0x2309bd52, 0x7b1c6f0b, 0x23491783, 0x7b0a55a1, 
  0x2388a1c4, 0x7af80b07, 0x23c85bbf, 0x7ae58ef5, 0x2408451a, 0x7ad2e124, 0x24485d7c, 0x7ac0014e, 
  0x2488a48a, 0x7aacef2e, 0x24c919e9, 0x7a99aa7e, 0x2509bd3d, 0x7a8632f8, 0x254a8e29, 0x7a728858, 
  0x258b8c50, 0x7a5eaa5a, 0x25ccb753, 0x7a4a98b9, 0x260e0ed3, 0x7a365333, 0x264f9271, 0x7a21d983, 
  0x269141cb, 0x7a0d2b68, 0x26d31c80, 0x79f8489e, 0x2715222f, 0x79e330e4, 0x27575273, 0x79cde3f8, 
  0x2799acea, 0x79b8619a, 0x27dc3130, 0x79a2a989, 0x281ededf, 0x798cbb85, 0x2861b591, 0x7976974e, 
  0x28a4b4e0, 0x79603ca5, 0x28e7dc65, 0x7949ab4c, 0x292b2bb8, 0x7932e304, 0x296ea270, 0x791be390, 
  0x29b24024, 0x7904acb3, 0x29f6046b, 0x78ed3e30, 0x2a39eed8, 0x78d597cc, 0x2a7dff02, 0x78bdb94a, 
  0x2ac2347c, 0x78a5a270, 0x2b068eda, 0x788d5304, 0x2b4b0dae, 0x7874cacb, 0x2b8fb08a, 0x785c098d, 
  0x2bd47700, 0x78430f11, 0x2c1960a1, 0x7829db1f, 0x2c5e6cfd, 0x78106d7f, 0x2ca39ba3, 0x77f6c5fb, 
  0x2ce8ec23, 0x77dce45c, 0x2d2e5e0b, 0x77c2c86e, 0x2d73f0e8, 0x77a871fa, 0x2db9a449, 0x778de0cd, 
  0x2dff77b8, 0x777314b2, 0x2e456ac4, 0x77580d78, 0x2e8b7cf6, 0x773ccaeb, 0x2ed1addb, 0x77214cdb, 
  0x2f17fcfb, 0x77059315, 0x2f5e69e2, 0x76e99d69, 0x2fa4f419, 0x76cd6ba9, 0x2feb9b27, 0x76b0fda4, 
  0x30325e96, 0x7694532e, 0x30793dee, 0x76776c17, 0x30c038b5, 0x765a4834, 0x31074e72, 0x763ce759, 
  0x314e7eab, 0x761f4959, 0x3195c8e6, 0x76016e0b, 0x31dd2ca9, 0x75e35545, 0x3224a979, 0x75c4fedc, 
  0x326c3ed8, 0x75a66aab, 0x32b3ec4d, 0x75879887, 0x32fbb159, 0x7568884b, 0x33438d81, 0x754939d1, 
  0x338b8045, 0x7529acf4, 0x33d3892a, 0x7509e18e, 0x341ba7b1, 0x74e9d77d, 0x3463db5a, 0x74c98e9e, 
  0x34ac23a7, 0x74a906cd, 0x34f48019, 0x74883fec, 0x353cf02f, 0x746739d8, 0x3585736a, 0x7445f472, 
  0x35ce0949, 0x74246f9c, 0x3616b14c, 0x7402ab37, 0x365f6af0, 0x73e0a727, 0x36a835b5, 0x73be6350, 
  0x36f11118, 0x739bdf95, 0x3739fc98, 0x73791bdd, 0x3782f7b2, 0x7356180e, 0x37cc01e3, 0x7332d410, 
  0x38151aa8, 0x730f4fc9, 0x385e417e, 0x72eb8b24, 0x38a775e1, 0x72c7860a, 0x38f0b74d, 0x72a34066, 
  0x393a053e, 0x727eba24, 0x39835f30, 0x7259f331, 0x39ccc49e, 0x7234eb79, 0x3a163503, 0x720fa2eb, 
  0x3a5fafda, 0x71ea1977, 0x3aa9349e, 0x71c44f0c, 0x3af2c2ca, 0x719e439d, 0x3b3c59d7, 0x7177f71a, 
  0x3b85f940, 0x71516978, 0x3bcfa07e, 0x712a9aaa, 0x3c194f0d, 0x71038aa4, 0x3c630464, 0x70dc395e, 
  0x3cacbfff, 0x70b4a6cd, 0x3cf68155, 0x708cd2e9, 0x3d4047e1, 0x7064bdab, 0x3d8a131c, 0x703c670d, 
  0x3dd3e27e, 0x7013cf0a, 0x3e1db580, 0x6feaf59c, 0x3e678b9b, 0x6fc1dac1, 0x3eb16449, 0x6f987e76, 
  0x3efb3f01, 0x6f6ee0b9, 0x3f451b3d, 0x6f45018b, 0x3f8ef874, 0x6f1ae0eb, 0x3fd8d620, 0x6ef07edb, 
  0x4022b3b9, 0x6ec5db5d, 0x406c90b7, 0x6e9af675, 0x40b66c93, 0x6e6fd027, 0x410046c5, 0x6e446879, 
  0x414a1ec6, 0x6e18bf71, 0x4193f40d, 0x6decd517, 0x41ddc615, 0x6dc0a972, 0x42279455, 0x6d943c8d, 
  0x42715e45, 0x6d678e71, 0x42bb235f, 0x6d3a9f2a, 0x4304e31a, 0x6d0d6ec5, 0x434e9cf1, 0x6cdffd4f, 
  0x4398505b, 0x6cb24ad6, 0x43e1fcd1, 0x6c84576b, 0x442ba1cd, 0x6c56231c, 0x44753ec7, 0x6c27adfd, 
  0x44bed33a, 0x6bf8f81e, 0x45085e9d, 0x6bca0195, 0x4551e06b, 0x6b9aca75, 0x459b581e, 0x6b6b52d5, 
  0x45e4c52f, 0x6b3b9ac9, 0x462e2717, 0x6b0ba26b, 0x46777d52, 0x6adb69d3, 0x46c0c75a, 0x6aaaf11b, 
  0x470a04a9, 0x6a7a385c, 0x475334b9, 0x6a493fb3, 0x479c5707, 0x6a18073d, 0x47e56b0c, 0x69e68f17, 
  0x482e7045, 0x69b4d761, 0x4877662c, 0x6982e039, 0x48c04c3f, 0x6950a9c0, 0x490921f8, 0x691e341a, 
  0x4951e6d5, 0x68eb7f67, 0x499a9a51, 0x68b88bcd, 0x49e33beb, 0x68855970, 0x4a2bcb1f, 0x6851e875, 
  0x4a74476b, 0x681e3905, 0x4abcb04c, 0x67ea4b47, 0x4b050541, 0x67b61f63, 0x4b4d45c9, 0x6781b585, 
  0x4b957162, 0x674d0dd6, 0x4bdd878c, 0x67182883, 0x4c2587c6, 0x66e305b8, 0x4c6d7190, 0x66ada5a5, 
  0x4cb5446a, 0x66780878, 0x4cfcffd5, 0x66422e60, 0x4d44a353, 0x660c1790, 0x4d8c2e64, 0x65d5c439, 
  0x4dd3a08c, 0x659f348e, 0x4e1af94b, 0x656868c3, 0x4e623825, 0x6531610d, 0x4ea95c9d, 0x64fa1da3, 
  0x4ef06637, 0x64c29ebb, 0x4f375477, 0x648ae48d, 0x4f7e26e1, 0x6452ef53, 0x4fc4dcfb, 0x641abf46, 
  0x500b7649, 0x63e254a2, 0x5051f253, 0x63a9afa2, 0x5098509f, 0x6370d083, 0x50de90b3, 0x6337b784, 
  0x5124b218, 0x62fe64e3, 0x516ab455, 0x62c4d8e0, 0x51b096f3, 0x628b13bc, 0x51f6597b, 0x625115b8, 
  0x523bfb78, 0x6216df18, 0x52817c72, 0x61dc701f, 0x52c6dbf5, 0x61a1c912, 0x530c198d, 0x6166ea36, 
  0x535134c5, 0x612bd3d2, 0x53962d2a, 0x60f0862d, 0x53db024a, 0x60b50190, 0x541fb3b1, 0x60794644, 
  0x546440ef, 0x603d5494, 0x54a8a992, 0x60012cca, 0x54eced2b, 0x5fc4cf33, 0x55310b48, 0x5f883c1c, 
  0x5575037c, 0x5f4b73d2, 0x55b8d558, 0x5f0e76a5, 0x55fc806f, 0x5ed144e5, 0x56400452, 0x5e93dee1, 
  0x56836096, 0x5e5644ec, 0x56c694cf, 0x5e187757, 0x5709a092, 0x5dda7677, 0x574c8374, 0x5d9c429f, 
  0x578f3d0d, 0x5d5ddc24, 0x57d1ccf2, 0x5d1f435d, 0x581432bd, 0x5ce078a0, 0x58566e04, 0x5ca17c45, 
  0x58987e63, 0x5c624ea4, 0x58da6372, 0x5c22f016, 0x591c1ccc, 0x5be360f6, 0x595daa0d, 0x5ba3a19f, 
  0x599f0ad1, 0x5b63b26c, 0x59e03eb6, 0x5b2393ba, 0x5a214558, 0x5ae345e7, 0x5a621e56, 0x5aa2c951, 
};

static s32 AAC_SIN_WINDOW_OFFSET[AAC_NUM_IMDCT_SIZES] = {0, 128};

static s32 AAC_SIN_WINDOW[128 + 1024] = {
  /* 128 - format = Q31 * 2^0 */
  0x00c90f88, 0x7fff6216, 0x025b26d7, 0x7ffa72d1, 0x03ed26e6, 0x7ff09478, 0x057f0035, 0x7fe1c76b, 
  0x0710a345, 0x7fce0c3e, 0x08a2009a, 0x7fb563b3, 0x0a3308bd, 0x7f97cebd, 0x0bc3ac35, 0x7f754e80, 
  0x0d53db92, 0x7f4de451, 0x0ee38766, 0x7f2191b4, 0x1072a048, 0x7ef05860, 0x120116d5, 0x7eba3a39, 
  0x138edbb1, 0x7e7f3957, 0x151bdf86, 0x7e3f57ff, 0x16a81305, 0x7dfa98a8, 0x183366e9, 0x7db0fdf8, 
  0x19bdcbf3, 0x7d628ac6, 0x1b4732ef, 0x7d0f4218, 0x1ccf8cb3, 0x7cb72724, 0x1e56ca1e, 0x7c5a3d50, 
  0x1fdcdc1b, 0x7bf88830, 0x2161b3a0, 0x7b920b89, 0x22e541af, 0x7b26cb4f, 0x24677758, 0x7ab6cba4, 
  0x25e845b6, 0x7a4210d8, 0x27679df4, 0x79c89f6e, 0x28e5714b, 0x794a7c12, 0x2a61b101, 0x78c7aba2, 
  0x2bdc4e6f, 0x78403329, 0x2d553afc, 0x77b417df, 0x2ecc681e, 0x77235f2d, 0x3041c761, 0x768e0ea6, 
  0x31b54a5e, 0x75f42c0b, 0x3326e2c3, 0x7555bd4c, 0x34968250, 0x74b2c884, 0x36041ad9, 0x740b53fb, 
  0x376f9e46, 0x735f6626, 0x38d8fe93, 0x72af05a7, 0x3a402dd2, 0x71fa3949, 0x3ba51e29, 0x71410805, 
  0x3d07c1d6, 0x708378ff, 0x3e680b2c, 0x6fc19385, 0x3fc5ec98, 0x6efb5f12, 0x4121589b, 0x6e30e34a, 
  0x427a41d0, 0x6d6227fa, 0x43d09aed, 0x6c8f351c, 0x452456bd, 0x6bb812d1, 0x46756828, 0x6adcc964, 
  0x47c3c22f, 0x69fd614a, 0x490f57ee, 0x6919e320, 0x4a581c9e, 0x683257ab, 0x4b9e0390, 0x6746c7d8, 
  0x4ce10034, 0x66573cbb, 0x4e210617, 0x6563bf92, 0x4f5e08e3, 0x646c59bf, 0x5097fc5e, 0x637114cc, 
  0x51ced46e, 0x6271fa69, 0x53028518, 0x616f146c, 0x5433027d, 0x60686ccf, 0x556040e2, 0x5f5e0db3, 
  0x568a34a9, 0x5e50015d, 0x57b0d256, 0x5d3e5237, 0x58d40e8c, 0x5c290acc, 0x59f3de12, 0x5b1035cf, 
  /* 1024 - format = Q31 * 2^0 */
  0x001921fb, 0x7ffffd88, 0x004b65ee, 0x7fffe9cb, 0x007da9d4, 0x7fffc251, 0x00afeda8, 0x7fff8719, 
  0x00e23160, 0x7fff3824, 0x011474f6, 0x7ffed572, 0x0146b860, 0x7ffe5f03, 0x0178fb99, 0x7ffdd4d7, 
  0x01ab3e97, 0x7ffd36ee, 0x01dd8154, 0x7ffc8549, 0x020fc3c6, 0x7ffbbfe6, 0x024205e8, 0x7ffae6c7, 
  0x027447b0, 0x7ff9f9ec, 0x02a68917, 0x7ff8f954, 0x02d8ca16, 0x7ff7e500, 0x030b0aa4, 0x7ff6bcf0, 
  0x033d4abb, 0x7ff58125, 0x036f8a51, 0x7ff4319d, 0x03a1c960, 0x7ff2ce5b, 0x03d407df, 0x7ff1575d, 
  0x040645c7, 0x7fefcca4, 0x04388310, 0x7fee2e30, 0x046abfb3, 0x7fec7c02, 0x049cfba7, 0x7feab61a, 
  0x04cf36e5, 0x7fe8dc78, 0x05017165, 0x7fe6ef1c, 0x0533ab20, 0x7fe4ee06, 0x0565e40d, 0x7fe2d938, 
  0x05981c26, 0x7fe0b0b1, 0x05ca5361, 0x7fde7471, 0x05fc89b8, 0x7fdc247a, 0x062ebf22, 0x7fd9c0ca, 
  0x0660f398, 0x7fd74964, 0x06932713, 0x7fd4be46, 0x06c5598a, 0x7fd21f72, 0x06f78af6, 0x7fcf6ce8, 
  0x0729bb4e, 0x7fcca6a7, 0x075bea8c, 0x7fc9ccb2, 0x078e18a7, 0x7fc6df08, 0x07c04598, 0x7fc3dda9, 
  0x07f27157, 0x7fc0c896, 0x08249bdd, 0x7fbd9fd0, 0x0856c520, 0x7fba6357, 0x0888ed1b, 0x7fb7132b, 
  0x08bb13c5, 0x7fb3af4e, 0x08ed3916, 0x7fb037bf, 0x091f5d06, 0x7facac7f, 0x09517f8f, 0x7fa90d8e, 
  0x0983a0a7, 0x7fa55aee, 0x09b5c048, 0x7fa1949e, 0x09e7de6a, 0x7f9dbaa0, 0x0a19fb04, 0x7f99ccf4, 
  0x0a4c1610, 0x7f95cb9a, 0x0a7e2f85, 0x7f91b694, 0x0ab0475c, 0x7f8d8de1, 0x0ae25d8d, 0x7f895182, 
  0x0b147211, 0x7f850179, 0x0b4684df, 0x7f809dc5, 0x0b7895f0, 0x7f7c2668, 0x0baaa53b, 0x7f779b62, 
  0x0bdcb2bb, 0x7f72fcb4, 0x0c0ebe66, 0x7f6e4a5e, 0x0c40c835, 0x7f698461, 0x0c72d020, 0x7f64aabf, 
  0x0ca4d620, 0x7f5fbd77, 0x0cd6da2d, 0x7f5abc8a, 0x0d08dc3f, 0x7f55a7fa, 0x0d3adc4e, 0x7f507fc7, 
  0x0d6cda53, 0x7f4b43f2, 0x0d9ed646, 0x7f45f47b, 0x0dd0d01f, 0x7f409164, 0x0e02c7d7, 0x7f3b1aad, 
  0x0e34bd66, 0x7f359057, 0x0e66b0c3, 0x7f2ff263, 0x0e98a1e9, 0x7f2a40d2, 0x0eca90ce, 0x7f247ba5, 
  0x0efc7d6b, 0x7f1ea2dc, 0x0f2e67b8, 0x7f18b679, 0x0f604faf, 0x7f12b67c, 0x0f923546, 0x7f0ca2e7, 
  0x0fc41876, 0x7f067bba, 0x0ff5f938, 0x7f0040f6, 0x1027d784, 0x7ef9f29d, 0x1059b352, 0x7ef390ae, 
  0x108b8c9b, 0x7eed1b2c, 0x10bd6356, 0x7ee69217, 0x10ef377d, 0x7edff570, 0x11210907, 0x7ed94538, 
  0x1152d7ed, 0x7ed28171, 0x1184a427, 0x7ecbaa1a, 0x11b66dad, 0x7ec4bf36, 0x11e83478, 0x7ebdc0c6, 
  0x1219f880, 0x7eb6aeca, 0x124bb9be, 0x7eaf8943, 0x127d7829, 0x7ea85033, 0x12af33ba, 0x7ea1039b, 
  0x12e0ec6a, 0x7e99a37c, 0x1312a230, 0x7e922fd6, 0x13445505, 0x7e8aa8ac, 0x137604e2, 0x7e830dff, 
  0x13a7b1bf, 0x7e7b5fce, 0x13d95b93, 0x7e739e1d, 0x140b0258, 0x7e6bc8eb, 0x143ca605, 0x7e63e03b, 
  0x146e4694, 0x7e5be40c, 0x149fe3fc, 0x7e53d462, 0x14d17e36, 0x7e4bb13c, 0x1503153a, 0x7e437a9c, 
  0x1534a901, 0x7e3b3083, 0x15663982, 0x7e32d2f4, 0x1597c6b7, 0x7e2a61ed, 0x15c95097, 0x7e21dd73, 
  0x15fad71b, 0x7e194584, 0x162c5a3b, 0x7e109a24, 0x165dd9f0, 0x7e07db52, 0x168f5632, 0x7dff0911, 
  0x16c0cef9, 0x7df62362, 0x16f2443e, 0x7ded2a47, 0x1723b5f9, 0x7de41dc0, 0x17552422, 0x7ddafdce, 
  0x17868eb3, 0x7dd1ca75, 0x17b7f5a3, 0x7dc883b4, 0x17e958ea, 0x7dbf298d, 0x181ab881, 0x7db5bc02, 
  0x184c1461, 0x7dac3b15, 0x187d6c82, 0x7da2a6c6, 0x18aec0db, 0x7d98ff17, 0x18e01167, 0x7d8f4409, 
  0x19115e1c, 0x7d85759f, 0x1942a6f3, 0x7d7b93da, 0x1973ebe6, 0x7d719eba, 0x19a52ceb, 0x7d679642, 
  0x19d669fc, 0x7d5d7a74, 0x1a07a311, 0x7d534b50, 0x1a38d823, 0x7d4908d9, 0x1a6a0929, 0x7d3eb30f, 
  0x1a9b361d, 0x7d3449f5, 0x1acc5ef6, 0x7d29cd8c, 0x1afd83ad, 0x7d1f3dd6, 0x1b2ea43a, 0x7d149ad5, 
  0x1b5fc097, 0x7d09e489, 0x1b90d8bb, 0x7cff1af5, 0x1bc1ec9e, 0x7cf43e1a, 0x1bf2fc3a, 0x7ce94dfb, 
  0x1c240786, 0x7cde4a98, 0x1c550e7c, 0x7cd333f3, 0x1c861113, 0x7cc80a0f, 0x1cb70f43, 0x7cbcccec, 
  0x1ce80906, 0x7cb17c8d, 0x1d18fe54, 0x7ca618f3, 0x1d49ef26, 0x7c9aa221, 0x1d7adb73, 0x7c8f1817, 
  0x1dabc334, 0x7c837ad8, 0x1ddca662, 0x7c77ca65, 0x1e0d84f5, 0x7c6c06c0, 0x1e3e5ee5, 0x7c602fec, 
  0x1e6f342c, 0x7c5445e9, 0x1ea004c1, 0x7c4848ba, 0x1ed0d09d, 0x7c3c3860, 0x1f0197b8, 0x7c3014de, 
  0x1f325a0b, 0x7c23de35, 0x1f63178f, 0x7c179467, 0x1f93d03c, 0x7c0b3777, 0x1fc4840a, 0x7bfec765, 
  0x1ff532f2, 0x7bf24434, 0x2025dcec, 0x7be5ade6, 0x205681f1, 0x7bd9047c, 0x208721f9, 0x7bcc47fa, 
  0x20b7bcfe, 0x7bbf7860, 0x20e852f6, 0x7bb295b0, 0x2118e3dc, 0x7ba59fee, 0x21496fa7, 0x7b989719, 
  0x2179f64f, 0x7b8b7b36, 0x21aa77cf, 0x7b7e4c45, 0x21daf41d, 0x7b710a49, 0x220b6b32, 0x7b63b543, 
  0x223bdd08, 0x7b564d36, 0x226c4996, 0x7b48d225, 0x229cb0d5, 0x7b3b4410, 0x22cd12bd, 0x7b2da2fa, 
  0x22fd6f48, 0x7b1feee5, 0x232dc66d, 0x7b1227d3, 0x235e1826, 0x7b044dc7, 0x238e646a, 0x7af660c2, 
  0x23beab33, 0x7ae860c7, 0x23eeec78, 0x7ada4dd8, 0x241f2833, 0x7acc27f7, 0x244f5e5c, 0x7abdef25, 
  0x247f8eec, 0x7aafa367, 0x24afb9da, 0x7aa144bc, 0x24dfdf20, 0x7a92d329, 0x250ffeb7, 0x7a844eae, 
  0x25401896, 0x7a75b74f, 0x25702cb7, 0x7a670d0d, 0x25a03b11, 0x7a584feb, 0x25d0439f, 0x7a497feb, 
  0x26004657, 0x7a3a9d0f, 0x26304333, 0x7a2ba75a, 0x26603a2c, 0x7a1c9ece, 0x26902b39, 0x7a0d836d, 
  0x26c01655, 0x79fe5539, 0x26effb76, 0x79ef1436, 0x271fda96, 0x79dfc064, 0x274fb3ae, 0x79d059c8, 
  0x277f86b5, 0x79c0e062, 0x27af53a6, 0x79b15435, 0x27df1a77, 0x79a1b545, 0x280edb23, 0x79920392, 
  0x283e95a1, 0x79823f20, 0x286e49ea, 0x797267f2, 0x289df7f8, 0x79627e08, 0x28cd9fc1, 0x79528167, 
  0x28fd4140, 0x79427210, 0x292cdc6d, 0x79325006, 0x295c7140, 0x79221b4b, 0x298bffb2, 0x7911d3e2, 
  0x29bb87bc, 0x790179cd, 0x29eb0957, 0x78f10d0f, 0x2a1a847b, 0x78e08dab, 0x2a49f920, 0x78cffba3, 
  0x2a796740, 0x78bf56f9, 0x2aa8ced3, 0x78ae9fb0, 0x2ad82fd2, 0x789dd5cb, 0x2b078a36, 0x788cf94c, 
  0x2b36ddf7, 0x787c0a36, 0x2b662b0e, 0x786b088c, 0x2b957173, 0x7859f44f, 0x2bc4b120, 0x7848cd83, 
  0x2bf3ea0d, 0x7837942b, 0x2c231c33, 0x78264849, 0x2c52478a, 0x7814e9df, 0x2c816c0c, 0x780378f1, 
  0x2cb089b1, 0x77f1f581, 0x2cdfa071, 0x77e05f91, 0x2d0eb046, 0x77ceb725, 0x2d3db928, 0x77bcfc3f, 
  0x2d6cbb10, 0x77ab2ee2, 0x2d9bb5f6, 0x77994f11, 0x2dcaa9d5, 0x77875cce, 0x2df996a3, 0x7775581d, 
  0x2e287c5a, 0x776340ff, 0x2e575af3, 0x77511778, 0x2e863267, 0x773edb8b, 0x2eb502ae, 0x772c8d3a, 
  0x2ee3cbc1, 0x771a2c88, 0x2f128d99, 0x7707b979, 0x2f41482e, 0x76f5340e, 0x2f6ffb7a, 0x76e29c4b, 
  0x2f9ea775, 0x76cff232, 0x2fcd4c19, 0x76bd35c7, 0x2ffbe95d, 0x76aa670d, 0x302a7f3a, 0x76978605, 
  0x30590dab, 0x768492b4, 0x308794a6, 0x76718d1c, 0x30b61426, 0x765e7540, 0x30e48c22, 0x764b4b23, 
  0x3112fc95, 0x76380ec8, 0x31416576, 0x7624c031, 0x316fc6be, 0x76115f63, 0x319e2067, 0x75fdec60, 
  0x31cc7269, 0x75ea672a, 0x31fabcbd, 0x75d6cfc5, 0x3228ff5c, 0x75c32634, 0x32573a3f, 0x75af6a7b, 
  0x32856d5e, 0x759b9c9b, 0x32b398b3, 0x7587bc98, 0x32e1bc36, 0x7573ca75, 0x330fd7e1, 0x755fc635, 
  0x333debab, 0x754bafdc, 0x336bf78f, 0x7537876c, 0x3399fb85, 0x75234ce8, 0x33c7f785, 0x750f0054, 
  0x33f5eb89, 0x74faa1b3, 0x3423d78a, 0x74e63108, 0x3451bb81, 0x74d1ae55, 0x347f9766, 0x74bd199f, 
  0x34ad6b32, 0x74a872e8, 0x34db36df, 0x7493ba34, 0x3508fa66, 0x747eef85, 0x3536b5be, 0x746a12df, 
  0x356468e2, 0x74552446, 0x359213c9, 0x744023bc, 0x35bfb66e, 0x742b1144, 0x35ed50c9, 0x7415ece2, 
  0x361ae2d3, 0x7400b69a, 0x36486c86, 0x73eb6e6e, 0x3675edd9, 0x73d61461, 0x36a366c6, 0x73c0a878, 
  0x36d0d746, 0x73ab2ab4, 0x36fe3f52, 0x73959b1b, 0x372b9ee3, 0x737ff9ae, 0x3758f5f2, 0x736a4671, 
  0x37864477, 0x73548168, 0x37b38a6d, 0x733eaa96, 0x37e0c7cc, 0x7328c1ff, 0x380dfc8d, 0x7312c7a5, 
  0x383b28a9, 0x72fcbb8c, 0x38684c19, 0x72e69db7, 0x389566d6, 0x72d06e2b, 0x38c278d9, 0x72ba2cea, 
  0x38ef821c, 0x72a3d9f7, 0x391c8297, 0x728d7557, 0x39497a43, 0x7276ff0d, 0x39766919, 0x7260771b, 
  0x39a34f13, 0x7249dd86, 0x39d02c2a, 0x72333251, 0x39fd0056, 0x721c7580, 0x3a29cb91, 0x7205a716, 
  0x3a568dd4, 0x71eec716, 0x3a834717, 0x71d7d585, 0x3aaff755, 0x71c0d265, 0x3adc9e86, 0x71a9bdba, 
  0x3b093ca3, 0x71929789, 0x3b35d1a5, 0x717b5fd3, 0x3b625d86, 0x7164169d, 0x3b8ee03e, 0x714cbbeb, 
  0x3bbb59c7, 0x71354fc0, 0x3be7ca1a, 0x711dd220, 0x3c143130, 0x7106430e, 0x3c408f03, 0x70eea28e, 
  0x3c6ce38a, 0x70d6f0a4, 0x3c992ec0, 0x70bf2d53, 0x3cc5709e, 0x70a7589f, 0x3cf1a91c, 0x708f728b, 
  0x3d1dd835, 0x70777b1c, 0x3d49fde1, 0x705f7255, 0x3d761a19, 0x70475839, 0x3da22cd7, 0x702f2ccd, 
  0x3dce3614, 0x7016f014, 0x3dfa35c8, 0x6ffea212, 0x3e262bee, 0x6fe642ca, 0x3e52187f, 0x6fcdd241, 
  0x3e7dfb73, 0x6fb5507a, 0x3ea9d4c3, 0x6f9cbd79, 0x3ed5a46b, 0x6f841942, 0x3f016a61, 0x6f6b63d8, 
  0x3f2d26a0, 0x6f529d40, 0x3f58d921, 0x6f39c57d, 0x3f8481dd, 0x6f20dc92, 0x3fb020ce, 0x6f07e285, 
  0x3fdbb5ec, 0x6eeed758, 0x40074132, 0x6ed5bb10, 0x4032c297, 0x6ebc8db0, 0x405e3a16, 0x6ea34f3d, 
  0x4089a7a8, 0x6e89ffb9, 0x40b50b46, 0x6e709f2a, 0x40e064ea, 0x6e572d93, 0x410bb48c, 0x6e3daaf8, 
  0x4136fa27, 0x6e24175c, 0x416235b2, 0x6e0a72c5, 0x418d6729, 0x6df0bd35, 0x41b88e84, 0x6dd6f6b1, 
  0x41e3abbc, 0x6dbd1f3c, 0x420ebecb, 0x6da336dc, 0x4239c7aa, 0x6d893d93, 0x4264c653, 0x6d6f3365, 
  0x428fbabe, 0x6d551858, 0x42baa4e6, 0x6d3aec6e, 0x42e584c3, 0x6d20afac, 0x43105a50, 0x6d066215, 
  0x433b2585, 0x6cec03af, 0x4365e65b, 0x6cd1947c, 0x43909ccd, 0x6cb71482, 0x43bb48d4, 0x6c9c83c3, 
  0x43e5ea68, 0x6c81e245, 0x44108184, 0x6c67300b, 0x443b0e21, 0x6c4c6d1a, 0x44659039, 0x6c319975, 
  0x449007c4, 0x6c16b521, 0x44ba74bd, 0x6bfbc021, 0x44e4d71c, 0x6be0ba7b, 0x450f2edb, 0x6bc5a431, 
  0x45397bf4, 0x6baa7d49, 0x4563be60, 0x6b8f45c7, 0x458df619, 0x6b73fdae, 0x45b82318, 0x6b58a503, 
  0x45e24556, 0x6b3d3bcb, 0x460c5cce, 0x6b21c208, 0x46366978, 0x6b0637c1, 0x46606b4e, 0x6aea9cf8, 
  0x468a624a, 0x6acef1b2, 0x46b44e65, 0x6ab335f4, 0x46de2f99, 0x6a9769c1, 0x470805df, 0x6a7b8d1e, 
  0x4731d131, 0x6a5fa010, 0x475b9188, 0x6a43a29a, 0x478546de, 0x6a2794c1, 0x47aef12c, 0x6a0b7689, 
  0x47d8906d, 0x69ef47f6, 0x48022499, 0x69d3090e, 0x482badab, 0x69b6b9d3, 0x48552b9b, 0x699a5a4c, 
  0x487e9e64, 0x697dea7b, 0x48a805ff, 0x69616a65, 0x48d16265, 0x6944da10, 0x48fab391, 0x6928397e, 
  0x4923f97b, 0x690b88b5, 0x494d341e, 0x68eec7b9, 0x49766373, 0x68d1f68f, 0x499f8774, 0x68b5153a, 
  0x49c8a01b, 0x689823bf, 0x49f1ad61, 0x687b2224, 0x4a1aaf3f, 0x685e106c, 0x4a43a5b0, 0x6840ee9b, 
  0x4a6c90ad, 0x6823bcb7, 0x4a957030, 0x68067ac3, 0x4abe4433, 0x67e928c5, 0x4ae70caf, 0x67cbc6c0, 
  0x4b0fc99d, 0x67ae54ba, 0x4b387af9, 0x6790d2b6, 0x4b6120bb, 0x677340ba, 0x4b89badd, 0x67559eca, 
  0x4bb24958, 0x6737ecea, 0x4bdacc28, 0x671a2b20, 0x4c034345, 0x66fc596f, 0x4c2baea9, 0x66de77dc, 
  0x4c540e4e, 0x66c0866d, 0x4c7c622d, 0x66a28524, 0x4ca4aa41, 0x66847408, 0x4ccce684, 0x6666531d, 
  0x4cf516ee, 0x66482267, 0x4d1d3b7a, 0x6629e1ec, 0x4d455422, 0x660b91af, 0x4d6d60df, 0x65ed31b5, 
  0x4d9561ac, 0x65cec204, 0x4dbd5682, 0x65b0429f, 0x4de53f5a, 0x6591b38c, 0x4e0d1c30, 0x657314cf, 
  0x4e34ecfc, 0x6554666d, 0x4e5cb1b9, 0x6535a86b, 0x4e846a60, 0x6516dacd, 0x4eac16eb, 0x64f7fd98, 
  0x4ed3b755, 0x64d910d1, 0x4efb4b96, 0x64ba147d, 0x4f22d3aa, 0x649b08a0, 0x4f4a4f89, 0x647bed3f, 
  0x4f71bf2e, 0x645cc260, 0x4f992293, 0x643d8806, 0x4fc079b1, 0x641e3e38, 0x4fe7c483, 0x63fee4f8, 
  0x500f0302, 0x63df7c4d, 0x50363529, 0x63c0043b, 0x505d5af1, 0x63a07cc7, 0x50847454, 0x6380e5f6, 
  0x50ab814d, 0x63613fcd, 0x50d281d5, 0x63418a50, 0x50f975e6, 0x6321c585, 0x51205d7b, 0x6301f171, 
  0x5147388c, 0x62e20e17, 0x516e0715, 0x62c21b7e, 0x5194c910, 0x62a219aa, 0x51bb7e75, 0x628208a1, 
  0x51e22740, 0x6261e866, 0x5208c36a, 0x6241b8ff, 0x522f52ee, 0x62217a72, 0x5255d5c5, 0x62012cc2, 
  0x527c4bea, 0x61e0cff5, 0x52a2b556, 0x61c06410, 0x52c91204, 0x619fe918, 0x52ef61ee, 0x617f5f12, 
  0x5315a50e, 0x615ec603, 0x533bdb5d, 0x613e1df0, 0x536204d7, 0x611d66de, 0x53882175, 0x60fca0d2, 
  0x53ae3131, 0x60dbcbd1, 0x53d43406, 0x60bae7e1, 0x53fa29ed, 0x6099f505, 0x542012e1, 0x6078f344, 
  0x5445eedb, 0x6057e2a2, 0x546bbdd7, 0x6036c325, 0x54917fce, 0x601594d1, 0x54b734ba, 0x5ff457ad, 
  0x54dcdc96, 0x5fd30bbc, 0x5502775c, 0x5fb1b104, 0x55280505, 0x5f90478a, 0x554d858d, 0x5f6ecf53, 
  0x5572f8ed, 0x5f4d4865, 0x55985f20, 0x5f2bb2c5, 0x55bdb81f, 0x5f0a0e77, 0x55e303e6, 0x5ee85b82, 
  0x5608426e, 0x5ec699e9, 0x562d73b2, 0x5ea4c9b3, 0x565297ab, 0x5e82eae5, 0x5677ae54, 0x5e60fd84, 
  0x569cb7a8, 0x5e3f0194, 0x56c1b3a1, 0x5e1cf71c, 0x56e6a239, 0x5dfade20, 0x570b8369, 0x5dd8b6a7, 
  0x5730572e, 0x5db680b4, 0x57551d80, 0x5d943c4e, 0x5779d65b, 0x5d71e979, 0x579e81b8, 0x5d4f883b, 
  0x57c31f92, 0x5d2d189a, 0x57e7afe4, 0x5d0a9a9a, 0x580c32a7, 0x5ce80e41, 0x5830a7d6, 0x5cc57394, 
  0x58550f6c, 0x5ca2ca99, 0x58796962, 0x5c801354, 0x589db5b3, 0x5c5d4dcc, 0x58c1f45b, 0x5c3a7a05, 
  0x58e62552, 0x5c179806, 0x590a4893, 0x5bf4a7d2, 0x592e5e19, 0x5bd1a971, 0x595265df, 0x5bae9ce7, 
  0x59765fde, 0x5b8b8239, 0x599a4c12, 0x5b68596d, 0x59be2a74, 0x5b452288, 0x59e1faff, 0x5b21dd90, 
  0x5a05bdae, 0x5afe8a8b, 0x5a29727b, 0x5adb297d, 0x5a4d1960, 0x5ab7ba6c, 0x5a70b258, 0x5a943d5e, 
};


AAC_DEF void aac_coefs_dec_window_overlap(s32 *buf0,
					  s32 *overlap0,
					  u8 *output,
					  s32 channels,
					  u8 window_shape,
					  s32 prev_window_shape) {

  buf0 += (1024 >> 1);
  s32 *buf1 = buf0 - 1;
  s16 *pcm0 = (s16 *) output;
  s16 *pcm1 = pcm0 + (1024 - 1) * channels;
  s32 *overlap1 = overlap0 + 1024 - 1;

  s32 *window_prev;
  if(prev_window_shape == 1) {
    window_prev = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[1];
  } else {
    window_prev = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[1];
  }

  if(window_shape == prev_window_shape) {
    do{
      s32 w0 = *window_prev++;
      s32 w1 = *window_prev++;
      s32 in = *buf0++;

      s32 f0 = aac_mulshift_32(w0, in);
      s32 f1 = aac_mulshift_32(w1, in);

      in = *overlap0;
      *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
      pcm0 += channels;

      in = *overlap1;
      *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
      pcm1 -= channels;

      in = *buf1--;
      *overlap1-- = aac_mulshift_32(w0, in);
      *overlap0++ = aac_mulshift_32(w1, in);
      
    }while(overlap0 < overlap1);
    
  } else {

    s32 *window_curr;
    if(window_shape == 1) {
      window_curr = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[1];
    } else {
      window_curr = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[1];
    }

    do {
      s32 w0 = *window_prev++;
      s32 w1 = *window_prev++;
      s32 in = *buf0++;

      s32 f0 = aac_mulshift_32(w0, in);
      s32 f1 = aac_mulshift_32(w1, in);

      in = *overlap0;
      *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
      pcm0 += channels;

      in = *overlap1;
      *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
      pcm1 -= channels;

      w0 = *window_curr++;
      w1 = *window_curr++;
      in = *buf1--;

      *overlap1-- = aac_mulshift_32(w0, in);
      *overlap0++ = aac_mulshift_32(w1, in);
      
    }while(overlap0 < overlap1);
    
  }
  
}

AAC_DEF void aac_coefs_dec_window_overlap_long_start(s32 *coefs,
						     s32 *overlap0,
						     u8 *output,
						     s32 channels,
						     u8 window_shape,
						     s32 prev_window_shape) {

  s32 *buf0 = coefs + (1024 >> 1);
  s32 *buf1 = buf0 - 1;
  s16 *pcm0 = (s16 *) output;
  s16 *pcm1 = pcm0 + (1024 - 1) * channels;
  s32 *overlap1 = overlap0 + 1024 - 1;

  s32 *window_prev;
  if(prev_window_shape == 1) {
    window_prev = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[1];
  } else {
    window_prev = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[1];
  }

  for(s32 i=448;i!=0;i--) {
    s32 w0 = *window_prev++;
    s32 w1 = *window_prev++;
    s32 in = *buf0++;

    s32 f0 = aac_mulshift_32(w0, in);
    s32 f1 = aac_mulshift_32(w1, in);

    in = *overlap0;
    *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;

    in = *overlap1;
    *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm1 -= channels;

    in = *buf1--;

    *overlap1-- = 0;
    *overlap0++ = in >> 1;
  }
  
  s32 *window_current;
  if(window_shape == 1) {
    window_current = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[0];
  } else {
    window_current = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[0];
  }

  do {
    s32 w0 = *window_prev++;
    s32 w1 = *window_prev++;
    s32 in = *buf0++;

    s32 f0 = aac_mulshift_32(w0, in);
    s32 f1 = aac_mulshift_32(w1, in);

    in = *overlap0;
    *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;

    in = *overlap1;
    *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm1 -= channels;

    w0 = *window_current++;
    w1 = *window_current++;
    in = *buf1--;

    *overlap1-- = aac_mulshift_32(w0, in);
    *overlap0++ = aac_mulshift_32(w1, in);
  } while(overlap0 < overlap1);
  
}

AAC_DEF void aac_coefs_dec_window_overlap_short(s32 *coefs,
						s32 *over0,
						u8 *output,
						s32 channels,
						u8 window_shape,
						s32 prev_window_shape) {

  s32 *buf0 = coefs;
  s16 *pcm0 = (s16 *) output;

  s32 *window_prev;
  if(prev_window_shape == 1) {
    window_prev = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[0];
  } else {
    window_prev = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[0];
  }

  s32 *window_current;
  if(window_shape == 1) {
    window_current = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[0];
  } else {
    window_current = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[0];
  }
  
  s32 i = 448;
  do {
    s32 f0 = *over0++;
    s32 f1 = *over0++;
    *pcm0 = aac_clip_to_short( (f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;
    *pcm0 = aac_clip_to_short( (f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;
    i -= 2;
  } while(i);

  s16 *pcm1 = pcm0 + (128 - 1) * channels;
  s32 *over1 = over0 + 128 - 1;
  buf0 += 64;
  s32 *buf1 = buf0 - 1;

  do{
    s32 w0 = *window_prev++;
    s32 w1 = *window_prev++;
    s32 in = *buf0++;

    s32 f0 = aac_mulshift_32(w0, in);
    s32 f1 = aac_mulshift_32(w1, in);

    in = *over0;
    *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;

    in = *over1;
    *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm1 -= channels;

    w0 = *window_current++;
    w1 = *window_current++;
    in = *buf1--;

    *over1-- = aac_mulshift_32(w0, in);
    *over0++ = aac_mulshift_32(w1, in);
  }while(over0 < over1);

  for(i=0;i<3;i++) {
    pcm0 += 64 * channels;
    pcm1 = pcm0 + (128 - 1) * channels;
    over0 += 64;
    over1 = over0 + 128 - 1;
    buf0 += 64;
    buf1 = buf0 - 1;
    window_current -= 128;

    do{
      s32 w0 = *window_current++;
      s32 w1 = *window_current++;
      s32 in = *buf0++;

      s32 f0 = aac_mulshift_32(w0, in);
      s32 f1 = aac_mulshift_32(w1, in);

      in =  *(over0 - 128);
      in += *(over0 + 0);
      *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
      pcm0 += channels;

      in =  *(over1 - 128);
      in += *(over1 + 0);
      *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
      pcm1 -= channels;
      
      in = *buf1--;
      *over1-- = aac_mulshift_32(w0, in);
      *over0++ = aac_mulshift_32(w1, in);
    }while(over0 < over1);
  }

  pcm0 += 64 * channels;
  over0 -= 832;
  over1 = over0 + 128 - 1;
  buf0 += 64;
  buf1 = buf0 - 1;
  window_current -= 128;
  do{
    s32 w0 = *window_current++;
    s32 w1 = *window_current++;
    s32 in = *buf0++;

    s32 f0 = aac_mulshift_32(w0, in);
    s32 f1 = aac_mulshift_32(w1, in);

    in =  *(over0 + 768);
    in += *(over0 + 896);
    *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;

    in = *(over1 + 768);
    *(over1 - 128) = in + f1;

    in = *buf1--;
    *over1-- = aac_mulshift_32(w0, in);
    *over0++ = aac_mulshift_32(w1, in);
    
  }while(over0 < over1);

  for(i=0;i<3;i++) {
    over0 += 64;
    over1 = over0 + 128 - 1;
    buf0 += 64;
    buf1 = buf0 - 1;
    window_current -= 128;

    do{
      s32 w0 = *window_current++;
      s32 w1 = *window_current++;
      s32 in = *buf0++;

      s32 f0 = aac_mulshift_32(w0, in);
      s32 f1 = aac_mulshift_32(w1, in);

      *(over0 - 128) -= f0;
      *(over1 - 128) += f1;

      in = *buf1--;
      *over1-- = aac_mulshift_32(w0, in);
      *over0++ = aac_mulshift_32(w1, in);
    }while(over0 < over1);
  }

  over0 += 64;
  i = 448;
  do {
    *over0++ = 0;
    *over0++ = 0;
    *over0++ = 0;
    *over0++ = 0;
    i-=4;
  } while(i);
      
}

AAC_DEF void aac_coefs_dec_window_overlap_long_stop(s32 *coefs,
						    s32 *overlap0,
						    u8 *output,
						    s32 channels,
						    u8 window_shape,
						    s32 prev_window_shape) {

  s32 *buf0 = coefs + (1024 >> 1);
  s32 *buf1 = buf0 - 1;
  s16 *pcm0 = (s16 *) output;
  s16 *pcm1 = pcm0 + (1024 - 1) * channels;
  s32 *overlap1 = overlap0 + 1024 - 1;

  s32 *window_prev;
  if(prev_window_shape == 1) {
    window_prev = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[0];
  } else {
    window_prev = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[0];
  }

  s32 *window_current;
  if(window_shape == 1) {
    window_current = AAC_KBD_WINDOW + AAC_KBD_WINDOW_OFFSET[1];
  } else {
    window_current = AAC_SIN_WINDOW + AAC_SIN_WINDOW_OFFSET[1];
  }

  for(s32 i=448;i!=0;i--) {
    s32 in = *buf0++;
    s32 f1 = in >> 1;

    in = *overlap0;
    *pcm0 = aac_clip_to_short( (in + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;

    in = *overlap1;
    *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm1 -= channels;

    s32 w0 = *window_current++;
    s32 w1 = *window_current++;
    in = *buf1--;

    *overlap1-- = aac_mulshift_32(w0, in);
    *overlap0++ = aac_mulshift_32(w1, in);
  }

  do {
    s32 w0 = *window_prev++;
    s32 w1 = *window_prev++;
    s32 in = *buf0++;

    s32 f0 = aac_mulshift_32(w0, in);
    s32 f1 = aac_mulshift_32(w1, in);

    in = *overlap0;
    *pcm0 = aac_clip_to_short( (in - f0 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm0 += channels;

    in = *overlap1;
    *pcm1 = aac_clip_to_short( (in + f1 + AAC_RND_VAL) >> AAC_FBITS_OUT_IMDCT );
    pcm1 -= channels;

    w0 = *window_current++;
    w1 = *window_current++;
    in = *buf1--;

    *overlap1-- = aac_mulshift_32(w0, in);
    *overlap0++ = aac_mulshift_32(w1, in);
  } while(overlap0 < overlap1);
  
}

static s32 AAC_NMDCT_TAB[AAC_NUM_IMDCT_SIZES] = {128, 1024};

static s32 AAC_COS_4_SIN_4_TAB_OFFSET[AAC_NUM_IMDCT_SIZES] = {0, 128};

static s32 AAC_COS_4_SIN_4_TAB[128 + 1024] = {
  /* 128 - format = Q30 * 2^-7 */
  0xbf9bc731, 0xff9b783c, 0xbed5332c, 0xc002c697, 0xbe112251, 0xfe096c8d, 0xbd4f9c30, 0xc00f1c4a, 
  0xbc90a83f, 0xfc77ae5e, 0xbbd44dd9, 0xc0254e27, 0xbb1a9443, 0xfae67ba2, 0xba6382a6, 0xc04558c0, 
  0xb9af200f, 0xf9561237, 0xb8fd7373, 0xc06f3726, 0xb84e83ac, 0xf7c6afdc, 0xb7a25779, 0xc0a2e2e3, 
  0xb6f8f57c, 0xf6389228, 0xb652643e, 0xc0e05401, 0xb5aeaa2a, 0xf4abf67e, 0xb50dcd90, 0xc1278104, 
  0xb46fd4a4, 0xf3211a07, 0xb3d4c57c, 0xc1785ef4, 0xb33ca614, 0xf19839a6, 0xb2a77c49, 0xc1d2e158, 
  0xb2154dda, 0xf01191f3, 0xb186206b, 0xc236fa3b, 0xb0f9f981, 0xee8d5f29, 0xb070de82, 0xc2a49a2e, 
  0xafead4b9, 0xed0bdd25, 0xaf67e14f, 0xc31bb049, 0xaee80952, 0xeb8d475b, 0xae6b51ae, 0xc39c2a2f, 
  0xadf1bf34, 0xea11d8c8, 0xad7b5692, 0xc425f410, 0xad081c5a, 0xe899cbf1, 0xac9814fd, 0xc4b8f8ad, 
  0xac2b44cc, 0xe7255ad1, 0xabc1aff9, 0xc555215a, 0xab5b5a96, 0xe5b4bed8, 0xaaf84896, 0xc5fa5603, 
  0xaa987dca, 0xe44830dd, 0xaa3bfde3, 0xc6a87d2d, 0xa9e2cc73, 0xe2dfe917, 0xa98cece9, 0xc75f7bfe, 
  0xa93a6296, 0xe17c1f15, 0xa8eb30a7, 0xc81f363d, 0xa89f5a2b, 0xe01d09b4, 0xa856e20e, 0xc8e78e5b, 
  0xa811cb1b, 0xdec2df18, 0xa7d017fc, 0xc9b86572, 0xa791cb39, 0xdd6dd4a2, 0xa756e73a, 0xca919b4e, 
  0xa71f6e43, 0xdc1e1ee9, 0xa6eb6279, 0xcb730e70, 0xa6bac5dc, 0xdad3f1b1, 0xa68d9a4c, 0xcc5c9c14, 
  0xa663e188, 0xd98f7fe6, 0xa63d9d2b, 0xcd4e2037, 0xa61aceaf, 0xd850fb8e, 0xa5fb776b, 0xce47759a, 
  0xa5df9894, 0xd71895c9, 0xa5c7333e, 0xcf4875ca, 0xa5b2485a, 0xd5e67ec1, 0xa5a0d8b5, 0xd050f926, 
  0xa592e4fd, 0xd4bae5ab, 0xa5886dba, 0xd160d6e5, 0xa5817354, 0xd395f8ba, 0xa57df60f, 0xd277e518, 
  /* 1024 - format = Q30 * 2^-10 */
  0xbff3703e, 0xfff36f02, 0xbfda5824, 0xc0000b1a, 0xbfc149ed, 0xffc12b16, 0xbfa845a0, 0xc0003c74, 
  0xbf8f4b3e, 0xff8ee750, 0xbf765acc, 0xc0009547, 0xbf5d744e, 0xff5ca3d0, 0xbf4497c8, 0xc0011594, 
  0xbf2bc53d, 0xff2a60b4, 0xbf12fcb2, 0xc001bd5c, 0xbefa3e2a, 0xfef81e1d, 0xbee189a8, 0xc0028c9c, 
  0xbec8df32, 0xfec5dc28, 0xbeb03eca, 0xc0038356, 0xbe97a875, 0xfe939af5, 0xbe7f1c36, 0xc004a188, 
  0xbe669a10, 0xfe615aa3, 0xbe4e2209, 0xc005e731, 0xbe35b423, 0xfe2f1b50, 0xbe1d5062, 0xc0075452, 
  0xbe04f6cb, 0xfdfcdd1d, 0xbdeca760, 0xc008e8e8, 0xbdd46225, 0xfdcaa027, 0xbdbc2720, 0xc00aa4f3, 
  0xbda3f652, 0xfd98648d, 0xbd8bcfbf, 0xc00c8872, 0xbd73b36d, 0xfd662a70, 0xbd5ba15d, 0xc00e9364, 
  0xbd439995, 0xfd33f1ed, 0xbd2b9c17, 0xc010c5c7, 0xbd13a8e7, 0xfd01bb24, 0xbcfbc00a, 0xc0131f9b, 
  0xbce3e182, 0xfccf8634, 0xbccc0d53, 0xc015a0dd, 0xbcb44382, 0xfc9d533b, 0xbc9c8411, 0xc018498c, 
  0xbc84cf05, 0xfc6b2259, 0xbc6d2461, 0xc01b19a7, 0xbc558428, 0xfc38f3ac, 0xbc3dee5f, 0xc01e112b, 
  0xbc266309, 0xfc06c754, 0xbc0ee22a, 0xc0213018, 0xbbf76bc4, 0xfbd49d70, 0xbbdfffdd, 0xc024766a, 
  0xbbc89e77, 0xfba2761e, 0xbbb14796, 0xc027e421, 0xbb99fb3e, 0xfb70517d, 0xbb82b972, 0xc02b7939, 
  0xbb6b8235, 0xfb3e2fac, 0xbb54558d, 0xc02f35b1, 0xbb3d337b, 0xfb0c10cb, 0xbb261c04, 0xc0331986, 
  0xbb0f0f2b, 0xfad9f4f8, 0xbaf80cf4, 0xc03724b6, 0xbae11561, 0xfaa7dc52, 0xbaca2878, 0xc03b573f, 
  0xbab3463b, 0xfa75c6f8, 0xba9c6eae, 0xc03fb11d, 0xba85a1d4, 0xfa43b508, 0xba6edfb1, 0xc044324f, 
  0xba582849, 0xfa11a6a3, 0xba417b9e, 0xc048dad1, 0xba2ad9b5, 0xf9df9be6, 0xba144291, 0xc04daaa1, 
  0xb9fdb635, 0xf9ad94f0, 0xb9e734a4, 0xc052a1bb, 0xb9d0bde4, 0xf97b91e1, 0xb9ba51f6, 0xc057c01d, 
  0xb9a3f0de, 0xf94992d7, 0xb98d9aa0, 0xc05d05c3, 0xb9774f3f, 0xf91797f0, 0xb9610ebe, 0xc06272aa, 
  0xb94ad922, 0xf8e5a14d, 0xb934ae6d, 0xc06806ce, 0xb91e8ea3, 0xf8b3af0c, 0xb90879c7, 0xc06dc22e, 
  0xb8f26fdc, 0xf881c14b, 0xb8dc70e7, 0xc073a4c3, 0xb8c67cea, 0xf84fd829, 0xb8b093ea, 0xc079ae8c, 
  0xb89ab5e8, 0xf81df3c5, 0xb884e2e9, 0xc07fdf85, 0xb86f1af0, 0xf7ec143e, 0xb8595e00, 0xc08637a9, 
  0xb843ac1d, 0xf7ba39b3, 0xb82e0549, 0xc08cb6f5, 0xb818698a, 0xf7886442, 0xb802d8e0, 0xc0935d64, 
  0xb7ed5351, 0xf756940a, 0xb7d7d8df, 0xc09a2af3, 0xb7c2698e, 0xf724c92a, 0xb7ad0561, 0xc0a11f9d, 
  0xb797ac5b, 0xf6f303c0, 0xb7825e80, 0xc0a83b5e, 0xb76d1bd2, 0xf6c143ec, 0xb757e455, 0xc0af7e33, 
  0xb742b80d, 0xf68f89cb, 0xb72d96fd, 0xc0b6e815, 0xb7188127, 0xf65dd57d, 0xb7037690, 0xc0be7901, 
  0xb6ee773a, 0xf62c2721, 0xb6d98328, 0xc0c630f2, 0xb6c49a5e, 0xf5fa7ed4, 0xb6afbce0, 0xc0ce0fe3, 
  0xb69aeab0, 0xf5c8dcb6, 0xb68623d1, 0xc0d615cf, 0xb6716847, 0xf59740e5, 0xb65cb815, 0xc0de42b2, 
  0xb648133e, 0xf565ab80, 0xb63379c5, 0xc0e69686, 0xb61eebae, 0xf5341ca5, 0xb60a68fb, 0xc0ef1147, 
  0xb5f5f1b1, 0xf5029473, 0xb5e185d1, 0xc0f7b2ee, 0xb5cd255f, 0xf4d11308, 0xb5b8d05f, 0xc1007b77, 
  0xb5a486d2, 0xf49f9884, 0xb59048be, 0xc1096add, 0xb57c1624, 0xf46e2504, 0xb567ef08, 0xc1128119, 
  0xb553d36c, 0xf43cb8a7, 0xb53fc355, 0xc11bbe26, 0xb52bbec4, 0xf40b538b, 0xb517c5be, 0xc12521ff, 
  0xb503d845, 0xf3d9f5cf, 0xb4eff65c, 0xc12eac9d, 0xb4dc2007, 0xf3a89f92, 0xb4c85548, 0xc1385dfb, 
  0xb4b49622, 0xf37750f2, 0xb4a0e299, 0xc1423613, 0xb48d3ab0, 0xf3460a0d, 0xb4799e69, 0xc14c34df, 
  0xb4660dc8, 0xf314cb02, 0xb45288cf, 0xc1565a58, 0xb43f0f82, 0xf2e393ef, 0xb42ba1e4, 0xc160a678, 
  0xb4183ff7, 0xf2b264f2, 0xb404e9bf, 0xc16b193a, 0xb3f19f3e, 0xf2813e2a, 0xb3de6078, 0xc175b296, 
  0xb3cb2d70, 0xf2501fb5, 0xb3b80628, 0xc1807285, 0xb3a4eaa4, 0xf21f09b1, 0xb391dae6, 0xc18b5903, 
  0xb37ed6f1, 0xf1edfc3d, 0xb36bdec9, 0xc1966606, 0xb358f26f, 0xf1bcf777, 0xb34611e8, 0xc1a1998a, 
  0xb3333d36, 0xf18bfb7d, 0xb320745c, 0xc1acf386, 0xb30db75d, 0xf15b086d, 0xb2fb063b, 0xc1b873f5, 
  0xb2e860fa, 0xf12a1e66, 0xb2d5c79d, 0xc1c41ace, 0xb2c33a26, 0xf0f93d86, 0xb2b0b898, 0xc1cfe80a, 
  0xb29e42f6, 0xf0c865ea, 0xb28bd943, 0xc1dbdba3, 0xb2797b82, 0xf09797b2, 0xb26729b5, 0xc1e7f591, 
  0xb254e3e0, 0xf066d2fa, 0xb242aa05, 0xc1f435cc, 0xb2307c27, 0xf03617e2, 0xb21e5a49, 0xc2009c4e, 
  0xb20c446d, 0xf0056687, 0xb1fa3a97, 0xc20d290d, 0xb1e83cc9, 0xefd4bf08, 0xb1d64b06, 0xc219dc03, 
  0xb1c46551, 0xefa42181, 0xb1b28bad, 0xc226b528, 0xb1a0be1b, 0xef738e12, 0xb18efca0, 0xc233b473, 
  0xb17d473d, 0xef4304d8, 0xb16b9df6, 0xc240d9de, 0xb15a00cd, 0xef1285f2, 0xb1486fc5, 0xc24e255e, 
  0xb136eae1, 0xeee2117c, 0xb1257223, 0xc25b96ee, 0xb114058e, 0xeeb1a796, 0xb102a524, 0xc2692e83, 
  0xb0f150e9, 0xee81485c, 0xb0e008e0, 0xc276ec16, 0xb0cecd09, 0xee50f3ed, 0xb0bd9d6a, 0xc284cf9f, 
  0xb0ac7a03, 0xee20aa67, 0xb09b62d8, 0xc292d914, 0xb08a57eb, 0xedf06be6, 0xb079593f, 0xc2a1086d, 
  0xb06866d7, 0xedc0388a, 0xb05780b5, 0xc2af5da2, 0xb046a6db, 0xed901070, 0xb035d94e, 0xc2bdd8a9, 
  0xb025180e, 0xed5ff3b5, 0xb014631e, 0xc2cc7979, 0xb003ba82, 0xed2fe277, 0xaff31e3b, 0xc2db400a, 
  0xafe28e4d, 0xecffdcd4, 0xafd20ab9, 0xc2ea2c53, 0xafc19383, 0xeccfe2ea, 0xafb128ad, 0xc2f93e4a, 
  0xafa0ca39, 0xec9ff4d6, 0xaf90782a, 0xc30875e5, 0xaf803283, 0xec7012b5, 0xaf6ff945, 0xc317d31c, 
  0xaf5fcc74, 0xec403ca5, 0xaf4fac12, 0xc32755e5, 0xaf3f9822, 0xec1072c4, 0xaf2f90a5, 0xc336fe37, 
  0xaf1f959f, 0xebe0b52f, 0xaf0fa712, 0xc346cc07, 0xaeffc500, 0xebb10404, 0xaeefef6c, 0xc356bf4d, 
  0xaee02658, 0xeb815f60, 0xaed069c7, 0xc366d7fd, 0xaec0b9bb, 0xeb51c760, 0xaeb11636, 0xc377160f, 
  0xaea17f3b, 0xeb223c22, 0xae91f4cd, 0xc3877978, 0xae8276ed, 0xeaf2bdc3, 0xae73059f, 0xc398022f, 
  0xae63a0e3, 0xeac34c60, 0xae5448be, 0xc3a8b028, 0xae44fd31, 0xea93e817, 0xae35be3f, 0xc3b9835a, 
  0xae268be9, 0xea649105, 0xae176633, 0xc3ca7bba, 0xae084d1f, 0xea354746, 0xadf940ae, 0xc3db993e, 
  0xadea40e4, 0xea060af9, 0xaddb4dc2, 0xc3ecdbdc, 0xadcc674b, 0xe9d6dc3b, 0xadbd8d82, 0xc3fe4388, 
  0xadaec067, 0xe9a7bb28, 0xad9fffff, 0xc40fd037, 0xad914c4b, 0xe978a7dd, 0xad82a54c, 0xc42181e0, 
  0xad740b07, 0xe949a278, 0xad657d7c, 0xc4335877, 0xad56fcaf, 0xe91aab16, 0xad4888a0, 0xc44553f2, 
  0xad3a2153, 0xe8ebc1d3, 0xad2bc6ca, 0xc4577444, 0xad1d7907, 0xe8bce6cd, 0xad0f380c, 0xc469b963, 
  0xad0103db, 0xe88e1a20, 0xacf2dc77, 0xc47c2344, 0xace4c1e2, 0xe85f5be9, 0xacd6b41e, 0xc48eb1db, 
  0xacc8b32c, 0xe830ac45, 0xacbabf10, 0xc4a1651c, 0xacacd7cb, 0xe8020b52, 0xac9efd60, 0xc4b43cfd, 
  0xac912fd1, 0xe7d3792b, 0xac836f1f, 0xc4c73972, 0xac75bb4d, 0xe7a4f5ed, 0xac68145d, 0xc4da5a6f, 
  0xac5a7a52, 0xe77681b6, 0xac4ced2c, 0xc4ed9fe7, 0xac3f6cef, 0xe7481ca1, 0xac31f99d, 0xc50109d0, 
  0xac249336, 0xe719c6cb, 0xac1739bf, 0xc514981d, 0xac09ed38, 0xe6eb8052, 0xabfcada3, 0xc5284ac3, 
  0xabef7b04, 0xe6bd4951, 0xabe2555b, 0xc53c21b4, 0xabd53caa, 0xe68f21e5, 0xabc830f5, 0xc5501ce5, 
  0xabbb323c, 0xe6610a2a, 0xabae4082, 0xc5643c4a, 0xaba15bc9, 0xe633023e, 0xab948413, 0xc5787fd6, 
  0xab87b962, 0xe6050a3b, 0xab7afbb7, 0xc58ce77c, 0xab6e4b15, 0xe5d72240, 0xab61a77d, 0xc5a17330, 
  0xab5510f3, 0xe5a94a67, 0xab488776, 0xc5b622e6, 0xab3c0b0b, 0xe57b82cd, 0xab2f9bb1, 0xc5caf690, 
  0xab23396c, 0xe54dcb8f, 0xab16e43d, 0xc5dfee22, 0xab0a9c27, 0xe52024c9, 0xaafe612a, 0xc5f5098f, 
  0xaaf23349, 0xe4f28e96, 0xaae61286, 0xc60a48c9, 0xaad9fee3, 0xe4c50914, 0xaacdf861, 0xc61fabc4, 
  0xaac1ff03, 0xe497945d, 0xaab612ca, 0xc6353273, 0xaaaa33b8, 0xe46a308f, 0xaa9e61cf, 0xc64adcc7, 
  0xaa929d10, 0xe43cddc4, 0xaa86e57e, 0xc660aab5, 0xaa7b3b1b, 0xe40f9c1a, 0xaa6f9de7, 0xc6769c2e, 
  0xaa640de6, 0xe3e26bac, 0xaa588b18, 0xc68cb124, 0xaa4d157f, 0xe3b54c95, 0xaa41ad1e, 0xc6a2e98b, 
  0xaa3651f6, 0xe3883ef2, 0xaa2b0409, 0xc6b94554, 0xaa1fc358, 0xe35b42df, 0xaa148fe6, 0xc6cfc472, 
  0xaa0969b3, 0xe32e5876, 0xa9fe50c2, 0xc6e666d7, 0xa9f34515, 0xe3017fd5, 0xa9e846ad, 0xc6fd2c75, 
  0xa9dd558b, 0xe2d4b916, 0xa9d271b2, 0xc714153e, 0xa9c79b23, 0xe2a80456, 0xa9bcd1e0, 0xc72b2123, 
  0xa9b215ea, 0xe27b61af, 0xa9a76744, 0xc7425016, 0xa99cc5ee, 0xe24ed13d, 0xa99231eb, 0xc759a20a, 
  0xa987ab3c, 0xe222531c, 0xa97d31e3, 0xc77116f0, 0xa972c5e1, 0xe1f5e768, 0xa9686738, 0xc788aeb9, 
  0xa95e15e9, 0xe1c98e3b, 0xa953d1f7, 0xc7a06957, 0xa9499b62, 0xe19d47b1, 0xa93f722c, 0xc7b846ba, 
  0xa9355658, 0xe17113e5, 0xa92b47e5, 0xc7d046d6, 0xa92146d7, 0xe144f2f3, 0xa917532e, 0xc7e8699a, 
  0xa90d6cec, 0xe118e4f6, 0xa9039413, 0xc800aef7, 0xa8f9c8a4, 0xe0ecea09, 0xa8f00aa0, 0xc81916df, 
  0xa8e65a0a, 0xe0c10247, 0xa8dcb6e2, 0xc831a143, 0xa8d3212a, 0xe0952dcb, 0xa8c998e3, 0xc84a4e14, 
  0xa8c01e10, 0xe0696cb0, 0xa8b6b0b1, 0xc8631d42, 0xa8ad50c8, 0xe03dbf11, 0xa8a3fe57, 0xc87c0ebd, 
  0xa89ab95e, 0xe012250a, 0xa89181df, 0xc8952278, 0xa88857dc, 0xdfe69eb4, 0xa87f3b57, 0xc8ae5862, 
  0xa8762c4f, 0xdfbb2c2c, 0xa86d2ac8, 0xc8c7b06b, 0xa86436c2, 0xdf8fcd8b, 0xa85b503e, 0xc8e12a84, 
  0xa852773f, 0xdf6482ed, 0xa849abc4, 0xc8fac69e, 0xa840edd1, 0xdf394c6b, 0xa8383d66, 0xc91484a8, 
  0xa82f9a84, 0xdf0e2a22, 0xa827052d, 0xc92e6492, 0xa81e7d62, 0xdee31c2b, 0xa8160324, 0xc948664d, 
  0xa80d9675, 0xdeb822a1, 0xa8053756, 0xc96289c9, 0xa7fce5c9, 0xde8d3d9e, 0xa7f4a1ce, 0xc97ccef5, 
  0xa7ec6b66, 0xde626d3e, 0xa7e44294, 0xc99735c2, 0xa7dc2759, 0xde37b199, 0xa7d419b4, 0xc9b1be1e, 
  0xa7cc19a9, 0xde0d0acc, 0xa7c42738, 0xc9cc67fa, 0xa7bc4262, 0xdde278ef, 0xa7b46b29, 0xc9e73346, 
  0xa7aca18e, 0xddb7fc1e, 0xa7a4e591, 0xca021fef, 0xa79d3735, 0xdd8d9472, 0xa795967a, 0xca1d2de7, 
  0xa78e0361, 0xdd634206, 0xa7867dec, 0xca385d1d, 0xa77f061c, 0xdd3904f4, 0xa7779bf2, 0xca53ad7e, 
  0xa7703f70, 0xdd0edd55, 0xa768f095, 0xca6f1efc, 0xa761af64, 0xdce4cb44, 0xa75a7bdd, 0xca8ab184, 
  0xa7535602, 0xdcbacedb, 0xa74c3dd4, 0xcaa66506, 0xa7453353, 0xdc90e834, 0xa73e3681, 0xcac23971, 
  0xa7374760, 0xdc671768, 0xa73065ef, 0xcade2eb3, 0xa7299231, 0xdc3d5c91, 0xa722cc25, 0xcafa44bc, 
  0xa71c13ce, 0xdc13b7c9, 0xa715692c, 0xcb167b79, 0xa70ecc41, 0xdbea292b, 0xa7083d0d, 0xcb32d2da, 
  0xa701bb91, 0xdbc0b0ce, 0xa6fb47ce, 0xcb4f4acd, 0xa6f4e1c6, 0xdb974ece, 0xa6ee8979, 0xcb6be341, 
  0xa6e83ee8, 0xdb6e0342, 0xa6e20214, 0xcb889c23, 0xa6dbd2ff, 0xdb44ce46, 0xa6d5b1a9, 0xcba57563, 
  0xa6cf9e13, 0xdb1baff2, 0xa6c9983e, 0xcbc26eee, 0xa6c3a02b, 0xdaf2a860, 0xa6bdb5da, 0xcbdf88b3, 
  0xa6b7d94e, 0xdac9b7a9, 0xa6b20a86, 0xcbfcc29f, 0xa6ac4984, 0xdaa0dde7, 0xa6a69649, 0xcc1a1ca0, 
  0xa6a0f0d5, 0xda781b31, 0xa69b5929, 0xcc3796a5, 0xa695cf46, 0xda4f6fa3, 0xa690532d, 0xcc55309b, 
  0xa68ae4df, 0xda26db54, 0xa685845c, 0xcc72ea70, 0xa68031a6, 0xd9fe5e5e, 0xa67aecbd, 0xcc90c412, 
  0xa675b5a3, 0xd9d5f8d9, 0xa6708c57, 0xccaebd6e, 0xa66b70db, 0xd9adaadf, 0xa6666330, 0xccccd671, 
  0xa6616355, 0xd9857489, 0xa65c714d, 0xcceb0f0a, 0xa6578d18, 0xd95d55ef, 0xa652b6b6, 0xcd096725, 
  0xa64dee28, 0xd9354f2a, 0xa6493370, 0xcd27deb0, 0xa644868d, 0xd90d6053, 0xa63fe781, 0xcd467599, 
  0xa63b564c, 0xd8e58982, 0xa636d2ee, 0xcd652bcb, 0xa6325d6a, 0xd8bdcad0, 0xa62df5bf, 0xcd840134, 
  0xa6299bed, 0xd8962456, 0xa6254ff7, 0xcda2f5c2, 0xa62111db, 0xd86e962b, 0xa61ce19c, 0xcdc20960, 
  0xa618bf39, 0xd8472069, 0xa614aab3, 0xcde13bfd, 0xa610a40c, 0xd81fc328, 0xa60cab43, 0xce008d84, 
  0xa608c058, 0xd7f87e7f, 0xa604e34e, 0xce1ffde2, 0xa6011424, 0xd7d15288, 0xa5fd52db, 0xce3f8d05, 
  0xa5f99f73, 0xd7aa3f5a, 0xa5f5f9ed, 0xce5f3ad8, 0xa5f2624a, 0xd783450d, 0xa5eed88a, 0xce7f0748, 
  0xa5eb5cae, 0xd75c63ba, 0xa5e7eeb6, 0xce9ef241, 0xa5e48ea3, 0xd7359b78, 0xa5e13c75, 0xcebefbb0, 
  0xa5ddf82d, 0xd70eec60, 0xa5dac1cb, 0xcedf2380, 0xa5d79950, 0xd6e85689, 0xa5d47ebc, 0xceff699f, 
  0xa5d17210, 0xd6c1da0b, 0xa5ce734d, 0xcf1fcdf8, 0xa5cb8272, 0xd69b76fe, 0xa5c89f80, 0xcf405077, 
  0xa5c5ca77, 0xd6752d79, 0xa5c30359, 0xcf60f108, 0xa5c04a25, 0xd64efd94, 0xa5bd9edc, 0xcf81af97, 
  0xa5bb017f, 0xd628e767, 0xa5b8720d, 0xcfa28c10, 0xa5b5f087, 0xd602eb0a, 0xa5b37cee, 0xcfc3865e, 
  0xa5b11741, 0xd5dd0892, 0xa5aebf82, 0xcfe49e6d, 0xa5ac75b0, 0xd5b74019, 0xa5aa39cd, 0xd005d42a, 
  0xa5a80bd7, 0xd59191b5, 0xa5a5ebd0, 0xd027277e, 0xa5a3d9b8, 0xd56bfd7d, 0xa5a1d590, 0xd0489856, 
  0xa59fdf57, 0xd5468389, 0xa59df70e, 0xd06a269d, 0xa59c1cb5, 0xd52123f0, 0xa59a504c, 0xd08bd23f, 
  0xa59891d4, 0xd4fbdec9, 0xa596e14e, 0xd0ad9b26, 0xa5953eb8, 0xd4d6b42b, 0xa593aa14, 0xd0cf813e, 
  0xa5922362, 0xd4b1a42c, 0xa590aaa2, 0xd0f18472, 0xa58f3fd4, 0xd48caee4, 0xa58de2f8, 0xd113a4ad, 
  0xa58c940f, 0xd467d469, 0xa58b5319, 0xd135e1d9, 0xa58a2016, 0xd44314d3, 0xa588fb06, 0xd1583be2, 
  0xa587e3ea, 0xd41e7037, 0xa586dac1, 0xd17ab2b3, 0xa585df8c, 0xd3f9e6ad, 0xa584f24b, 0xd19d4636, 
  0xa58412fe, 0xd3d5784a, 0xa58341a5, 0xd1bff656, 0xa5827e40, 0xd3b12526, 0xa581c8d0, 0xd1e2c2fd, 
  0xa5812154, 0xd38ced57, 0xa58087cd, 0xd205ac17, 0xa57ffc3b, 0xd368d0f3, 0xa57f7e9d, 0xd228b18d, 
  0xa57f0ef5, 0xd344d011, 0xa57ead41, 0xd24bd34a, 0xa57e5982, 0xd320eac6, 0xa57e13b8, 0xd26f1138, 
  0xa57ddbe4, 0xd2fd2129, 0xa57db204, 0xd2926b41, 0xa57d961a, 0xd2d97350, 0xa57d8825, 0xd2b5e151, 
};

AAC_DEF void aac_coefs_pre_multiply_rescale(s32 *coefs, s32 es, s32 tab) {

  s32 nmdct = AAC_NMDCT_TAB[tab];
  s32 *zbuf1 = coefs;
  s32 *zbuf2 = zbuf1 + nmdct - 1;
  s32 *csptr = AAC_COS_4_SIN_4_TAB + AAC_COS_4_SIN_4_TAB_OFFSET[tab];

  for(s32 i=nmdct>>2;i!=0;i--) {
    s32 cps2a = *csptr++;
    s32 sin2a = *csptr++;
    s32 cps2b = *csptr++;
    s32 sin2b = *csptr++;

    s32 ar1 = *(zbuf1 + 0) >> es;
    s32 ai1 = *(zbuf2 + 0) >> es;
    s32 ai2 = *(zbuf1 + 1) >> es;

    s32 t, z1, z2, cms2;

    t = aac_mulshift_32(sin2a, ar1 + ai1);
    z2 = aac_mulshift_32(cps2a, ai1) - t;
    cms2 = cps2a - 2*sin2a;
    z1 = aac_mulshift_32(cms2, ar1) + t;
    *zbuf1++ = z1;
    *zbuf1++ = z2;

    s32 ar2 = *(zbuf2 - 1) >> es;

    t = aac_mulshift_32(sin2b, ar2 + ai2);
    z2 = aac_mulshift_32(cps2b, ai2) - t;
    cms2 = cps2b - 2*sin2b;
    z1 = aac_mulshift_32(cms2, ar2) + t;
    *zbuf2-- = z2;
    *zbuf2-- = z1;
  }
  
}

static s32 AAC_NFFT_LOG_2_TAB[AAC_NUM_FFT_SIZES] = {6, 9};
static s32 AAC_NFFT_TAB[AAC_NUM_FFT_SIZES] = {64, 512};

static s32 AAC_BIT_REV_TAB_OFFSET[AAC_NUM_IMDCT_SIZES] = {0, 17};
static u8 AAC_BIT_REV_TAB[17 + 129] = {
  /* nfft = 64 */
  0x01, 0x08, 0x02, 0x04, 0x03, 0x0c, 0x05, 0x0a, 0x07, 0x0e, 0x0b, 0x0d, 0x00, 0x06, 0x09, 0x0f,
  0x00,

  /* nfft = 512 */
  0x01, 0x40, 0x02, 0x20, 0x03, 0x60, 0x04, 0x10, 0x05, 0x50, 0x06, 0x30, 0x07, 0x70, 0x09, 0x48,
  0x0a, 0x28, 0x0b, 0x68, 0x0c, 0x18, 0x0d, 0x58, 0x0e, 0x38, 0x0f, 0x78, 0x11, 0x44, 0x12, 0x24,
  0x13, 0x64, 0x15, 0x54, 0x16, 0x34, 0x17, 0x74, 0x19, 0x4c, 0x1a, 0x2c, 0x1b, 0x6c, 0x1d, 0x5c,
  0x1e, 0x3c, 0x1f, 0x7c, 0x21, 0x42, 0x23, 0x62, 0x25, 0x52, 0x26, 0x32, 0x27, 0x72, 0x29, 0x4a,
  0x2b, 0x6a, 0x2d, 0x5a, 0x2e, 0x3a, 0x2f, 0x7a, 0x31, 0x46, 0x33, 0x66, 0x35, 0x56, 0x37, 0x76,
  0x39, 0x4e, 0x3b, 0x6e, 0x3d, 0x5e, 0x3f, 0x7e, 0x43, 0x61, 0x45, 0x51, 0x47, 0x71, 0x4b, 0x69,
  0x4d, 0x59, 0x4f, 0x79, 0x53, 0x65, 0x57, 0x75, 0x5b, 0x6d, 0x5f, 0x7d, 0x67, 0x73, 0x6f, 0x7b,
  0x00, 0x08, 0x14, 0x1c, 0x22, 0x2a, 0x36, 0x3e, 0x41, 0x49, 0x55, 0x5d, 0x63, 0x6b, 0x77, 0x7f,
  0x00,
};

#define __aac_coefs_bitreverse_swapclx(p0,p1) \
  t = p0; t1 = *(&(p0)+1); p0 = p1; *(&(p0)+1) = *(&(p1)+1); p1 = t; *(&(p1)+1) = t1

AAC_DEF void aac_coefs_bitreverse(s32 *coefs, s32 _tab) {

  s32 nbits = AAC_NFFT_LOG_2_TAB[_tab];
  u8 *tab = AAC_BIT_REV_TAB + AAC_BIT_REV_TAB_OFFSET[_tab];
  
  s32 *part0 = coefs;
  s32 *part1 = coefs + (1 << nbits);

  s32 a, b, t, t1;
  while((a = *tab++) != 0) {
    b = *tab++;

    __aac_coefs_bitreverse_swapclx(part0[4*a+0], part0[4*b+0]);
    __aac_coefs_bitreverse_swapclx(part0[4*a+2], part1[4*b+0]);
    __aac_coefs_bitreverse_swapclx(part1[4*a+0], part0[4*b+2]);
    __aac_coefs_bitreverse_swapclx(part1[4*a+2], part1[4*b+2]);
  }

  do {
    __aac_coefs_bitreverse_swapclx(part0[4*a+2], part1[4*a+0]);
  } while((a = *tab++) != 0);
  
}

AAC_DEF void aac_coefs_r8_first_pass(s32 *x, s32 bg) {
  s32 ar, ai, br, bi, cr, ci, dr, di;
  s32 sr, si, tr, ti, ur, ui, vr, vi;
  s32 wr, wi, xr, xi, yr, yi, zr, zi;

  for (; bg != 0; bg--) {

    ar = x[0] + x[2];
    br = x[0] - x[2];
    ai = x[1] + x[3];
    bi = x[1] - x[3];
    cr = x[4] + x[6];
    dr = x[4] - x[6];
    ci = x[5] + x[7];
    di = x[5] - x[7];

    sr = ar + cr;
    ur = ar - cr;
    si = ai + ci;
    ui = ai - ci;
    tr = br - di;
    vr = br + di;
    ti = bi + dr;
    vi = bi - dr;

    ar = x[ 8] + x[10];
    br = x[ 8] - x[10];
    ai = x[ 9] + x[11];
    bi = x[ 9] - x[11];
    cr = x[12] + x[14];
    dr = x[12] - x[14];
    ci = x[13] + x[15];
    di = x[13] - x[15];

    wr = (ar + cr) >> 1;
    yr = (ar - cr) >> 1;
    wi = (ai + ci) >> 1;
    yi = (ai - ci) >> 1;

    x[ 0] = (sr >> 1) + wr;
    x[ 8] = (sr >> 1) - wr;
    x[ 1] = (si >> 1) + wi;
    x[ 9] = (si >> 1) - wi;
    x[ 4] = (ur >> 1) + yi;
    x[12] = (ur >> 1) - yi;
    x[ 5] = (ui >> 1) - yr;
    x[13] = (ui >> 1) + yr;

    ar = br - di;
    cr = br + di;
    ai = bi + dr;
    ci = bi - dr;

    xr = aac_mulshift_32(AAC_SQRTHALF, ar - ai);
    xi = aac_mulshift_32(AAC_SQRTHALF, ar + ai);
    zr = aac_mulshift_32(AAC_SQRTHALF, cr - ci);
    zi = aac_mulshift_32(AAC_SQRTHALF, cr + ci);

    x[ 6] = (tr >> 1) - xr;
    x[14] = (tr >> 1) + xr;
    x[ 7] = (ti >> 1) - xi;
    x[15] = (ti >> 1) + xi;
    x[ 2] = (vr >> 1) + zi;
    x[10] = (vr >> 1) - zi;
    x[ 3] = (vi >> 1) - zr;
    x[11] = (vi >> 1) + zr;

    x += 16;
  }

}

AAC_DEF void aac_coefs_r4_first_pass(s32 *coefs, s32 bg) {
  s32 ar, ai, br, bi, cr, ci, dr, di;
	
  for (; bg != 0; bg--) {

    ar = coefs[0] + coefs[2];
    br = coefs[0] - coefs[2];
    ai = coefs[1] + coefs[3];
    bi = coefs[1] - coefs[3];
    cr = coefs[4] + coefs[6];
    dr = coefs[4] - coefs[6];
    ci = coefs[5] + coefs[7];
    di = coefs[5] - coefs[7];

    coefs[0] = ar + cr;
    coefs[4] = ar - cr;
    coefs[1] = ai + ci;
    coefs[5] = ai - ci;
    coefs[2] = br + di;
    coefs[6] = br - di;
    coefs[3] = bi - dr;
    coefs[7] = bi + dr;

    coefs += 8;
  }
}

static s32 AAC_TWID_TAB_ODD[8*6 + 32*6 + 128*6] = {
  0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x539eba45, 0xe7821d59, 
  0x4b418bbe, 0xf383a3e2, 0x58c542c5, 0xdc71898d, 0x5a82799a, 0xd2bec333, 0x539eba45, 0xe7821d59, 
  0x539eba45, 0xc4df2862, 0x539eba45, 0xc4df2862, 0x58c542c5, 0xdc71898d, 0x3248d382, 0xc13ad060, 
  0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 0x00000000, 0xd2bec333, 0x22a2f4f8, 0xc4df2862, 
  0x58c542c5, 0xcac933ae, 0xcdb72c7e, 0xf383a3e2, 0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862, 
  0xac6145bb, 0x187de2a7, 0xdd5d0b08, 0xe7821d59, 0x4b418bbe, 0xc13ad060, 0xa73abd3b, 0x3536cc52, 
	
  0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x45f704f7, 0xf9ba1651, 
  0x43103085, 0xfcdc1342, 0x48b2b335, 0xf69bf7c9, 0x4b418bbe, 0xf383a3e2, 0x45f704f7, 0xf9ba1651, 
  0x4fd288dc, 0xed6bf9d1, 0x4fd288dc, 0xed6bf9d1, 0x48b2b335, 0xf69bf7c9, 0x553805f2, 0xe4a2eff6, 
  0x539eba45, 0xe7821d59, 0x4b418bbe, 0xf383a3e2, 0x58c542c5, 0xdc71898d, 0x569cc31b, 0xe1d4a2c8, 
  0x4da1fab5, 0xf0730342, 0x5a6690ae, 0xd5052d97, 0x58c542c5, 0xdc71898d, 0x4fd288dc, 0xed6bf9d1, 
  0x5a12e720, 0xce86ff2a, 0x5a12e720, 0xd76619b6, 0x51d1dc80, 0xea70658a, 0x57cc15bc, 0xc91af976, 
  0x5a82799a, 0xd2bec333, 0x539eba45, 0xe7821d59, 0x539eba45, 0xc4df2862, 0x5a12e720, 0xce86ff2a, 
  0x553805f2, 0xe4a2eff6, 0x4da1fab5, 0xc1eb0209, 0x58c542c5, 0xcac933ae, 0x569cc31b, 0xe1d4a2c8, 
  0x45f704f7, 0xc04ee4b8, 0x569cc31b, 0xc78e9a1d, 0x57cc15bc, 0xdf18f0ce, 0x3cc85709, 0xc013bc39, 
  0x539eba45, 0xc4df2862, 0x58c542c5, 0xdc71898d, 0x3248d382, 0xc13ad060, 0x4fd288dc, 0xc2c17d52, 
  0x5987b08a, 0xd9e01006, 0x26b2a794, 0xc3bdbdf6, 0x4b418bbe, 0xc13ad060, 0x5a12e720, 0xd76619b6, 
  0x1a4608ab, 0xc78e9a1d, 0x45f704f7, 0xc04ee4b8, 0x5a6690ae, 0xd5052d97, 0x0d47d096, 0xcc983f70, 
  0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 0x00000000, 0xd2bec333, 0x396b3199, 0xc04ee4b8, 
  0x5a6690ae, 0xd09441bb, 0xf2b82f6a, 0xd9e01006, 0x3248d382, 0xc13ad060, 0x5a12e720, 0xce86ff2a, 
  0xe5b9f755, 0xe1d4a2c8, 0x2aaa7c7f, 0xc2c17d52, 0x5987b08a, 0xcc983f70, 0xd94d586c, 0xea70658a, 
  0x22a2f4f8, 0xc4df2862, 0x58c542c5, 0xcac933ae, 0xcdb72c7e, 0xf383a3e2, 0x1a4608ab, 0xc78e9a1d, 
  0x57cc15bc, 0xc91af976, 0xc337a8f7, 0xfcdc1342, 0x11a855df, 0xcac933ae, 0x569cc31b, 0xc78e9a1d, 
  0xba08fb09, 0x0645e9af, 0x08df1a8c, 0xce86ff2a, 0x553805f2, 0xc6250a18, 0xb25e054b, 0x0f8cfcbe, 
  0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862, 0xac6145bb, 0x187de2a7, 0xf720e574, 0xd76619b6, 
  0x51d1dc80, 0xc3bdbdf6, 0xa833ea44, 0x20e70f32, 0xee57aa21, 0xdc71898d, 0x4fd288dc, 0xc2c17d52, 
  0xa5ed18e0, 0x2899e64a, 0xe5b9f755, 0xe1d4a2c8, 0x4da1fab5, 0xc1eb0209, 0xa5996f52, 0x2f6bbe45, 
  0xdd5d0b08, 0xe7821d59, 0x4b418bbe, 0xc13ad060, 0xa73abd3b, 0x3536cc52, 0xd5558381, 0xed6bf9d1, 
  0x48b2b335, 0xc0b15502, 0xaac7fa0e, 0x39daf5e8, 0xcdb72c7e, 0xf383a3e2, 0x45f704f7, 0xc04ee4b8, 
  0xb02d7724, 0x3d3e82ae, 0xc694ce67, 0xf9ba1651, 0x43103085, 0xc013bc39, 0xb74d4ccb, 0x3f4eaafe, 
	
  0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x418d2621, 0xfe6deaa1, 
  0x40c7d2bd, 0xff36f170, 0x424ff28f, 0xfda4f351, 0x43103085, 0xfcdc1342, 0x418d2621, 0xfe6deaa1, 
  0x4488e37f, 0xfb4ab7db, 0x4488e37f, 0xfb4ab7db, 0x424ff28f, 0xfda4f351, 0x46aa0d6d, 0xf8f21e8e, 
  0x45f704f7, 0xf9ba1651, 0x43103085, 0xfcdc1342, 0x48b2b335, 0xf69bf7c9, 0x475a5c77, 0xf82a6c6a, 
  0x43cdd89a, 0xfc135231, 0x4aa22036, 0xf4491311, 0x48b2b335, 0xf69bf7c9, 0x4488e37f, 0xfb4ab7db, 
  0x4c77a88e, 0xf1fa3ecb, 0x49ffd417, 0xf50ef5de, 0x454149fc, 0xfa824bfd, 0x4e32a956, 0xefb047f2, 
  0x4b418bbe, 0xf383a3e2, 0x45f704f7, 0xf9ba1651, 0x4fd288dc, 0xed6bf9d1, 0x4c77a88e, 0xf1fa3ecb, 
  0x46aa0d6d, 0xf8f21e8e, 0x5156b6d9, 0xeb2e1dbe, 0x4da1fab5, 0xf0730342, 0x475a5c77, 0xf82a6c6a, 
  0x52beac9f, 0xe8f77acf, 0x4ec05432, 0xeeee2d9d, 0x4807eb4b, 0xf7630799, 0x5409ed4b, 0xe6c8d59c, 
  0x4fd288dc, 0xed6bf9d1, 0x48b2b335, 0xf69bf7c9, 0x553805f2, 0xe4a2eff6, 0x50d86e6d, 0xebeca36c, 
  0x495aada2, 0xf5d544a7, 0x56488dc5, 0xe28688a4, 0x51d1dc80, 0xea70658a, 0x49ffd417, 0xf50ef5de, 
  0x573b2635, 0xe0745b24, 0x52beac9f, 0xe8f77acf, 0x4aa22036, 0xf4491311, 0x580f7b19, 0xde6d1f65, 
  0x539eba45, 0xe7821d59, 0x4b418bbe, 0xf383a3e2, 0x58c542c5, 0xdc71898d, 0x5471e2e6, 0xe61086bc, 
  0x4bde1089, 0xf2beafed, 0x595c3e2a, 0xda8249b4, 0x553805f2, 0xe4a2eff6, 0x4c77a88e, 0xf1fa3ecb, 
  0x59d438e5, 0xd8a00bae, 0x55f104dc, 0xe3399167, 0x4d0e4de2, 0xf136580d, 0x5a2d0957, 0xd6cb76c9, 
  0x569cc31b, 0xe1d4a2c8, 0x4da1fab5, 0xf0730342, 0x5a6690ae, 0xd5052d97, 0x573b2635, 0xe0745b24, 
  0x4e32a956, 0xefb047f2, 0x5a80baf6, 0xd34dcdb4, 0x57cc15bc, 0xdf18f0ce, 0x4ec05432, 0xeeee2d9d, 
  0x5a7b7f1a, 0xd1a5ef90, 0x584f7b58, 0xddc29958, 0x4f4af5d1, 0xee2cbbc1, 0x5a56deec, 0xd00e2639, 
  0x58c542c5, 0xdc71898d, 0x4fd288dc, 0xed6bf9d1, 0x5a12e720, 0xce86ff2a, 0x592d59da, 0xdb25f566, 
  0x50570819, 0xecabef3d, 0x59afaf4c, 0xcd110216, 0x5987b08a, 0xd9e01006, 0x50d86e6d, 0xebeca36c, 
  0x592d59da, 0xcbacb0bf, 0x59d438e5, 0xd8a00bae, 0x5156b6d9, 0xeb2e1dbe, 0x588c1404, 0xca5a86c4, 
  0x5a12e720, 0xd76619b6, 0x51d1dc80, 0xea70658a, 0x57cc15bc, 0xc91af976, 0x5a43b190, 0xd6326a88, 
  0x5249daa2, 0xe9b38223, 0x56eda1a0, 0xc7ee77b3, 0x5a6690ae, 0xd5052d97, 0x52beac9f, 0xe8f77acf, 
  0x55f104dc, 0xc6d569be, 0x5a7b7f1a, 0xd3de9156, 0x53304df6, 0xe83c56cf, 0x54d69714, 0xc5d03118, 
  0x5a82799a, 0xd2bec333, 0x539eba45, 0xe7821d59, 0x539eba45, 0xc4df2862, 0x5a7b7f1a, 0xd1a5ef90, 
  0x5409ed4b, 0xe6c8d59c, 0x5249daa2, 0xc402a33c, 0x5a6690ae, 0xd09441bb, 0x5471e2e6, 0xe61086bc, 
  0x50d86e6d, 0xc33aee27, 0x5a43b190, 0xcf89e3e8, 0x54d69714, 0xe55937d5, 0x4f4af5d1, 0xc2884e6e, 
  0x5a12e720, 0xce86ff2a, 0x553805f2, 0xe4a2eff6, 0x4da1fab5, 0xc1eb0209, 0x59d438e5, 0xcd8bbb6d, 
  0x55962bc0, 0xe3edb628, 0x4bde1089, 0xc1633f8a, 0x5987b08a, 0xcc983f70, 0x55f104dc, 0xe3399167, 
  0x49ffd417, 0xc0f1360b, 0x592d59da, 0xcbacb0bf, 0x56488dc5, 0xe28688a4, 0x4807eb4b, 0xc0950d1d, 
  0x58c542c5, 0xcac933ae, 0x569cc31b, 0xe1d4a2c8, 0x45f704f7, 0xc04ee4b8, 0x584f7b58, 0xc9edeb50, 
  0x56eda1a0, 0xe123e6ad, 0x43cdd89a, 0xc01ed535, 0x57cc15bc, 0xc91af976, 0x573b2635, 0xe0745b24, 
  0x418d2621, 0xc004ef3f, 0x573b2635, 0xc8507ea7, 0x57854ddd, 0xdfc606f1, 0x3f35b59d, 0xc0013bd3, 
  0x569cc31b, 0xc78e9a1d, 0x57cc15bc, 0xdf18f0ce, 0x3cc85709, 0xc013bc39, 0x55f104dc, 0xc6d569be, 
  0x580f7b19, 0xde6d1f65, 0x3a45e1f7, 0xc03c6a07, 0x553805f2, 0xc6250a18, 0x584f7b58, 0xddc29958, 
  0x37af354c, 0xc07b371e, 0x5471e2e6, 0xc57d965d, 0x588c1404, 0xdd196538, 0x350536f1, 0xc0d00db6, 
  0x539eba45, 0xc4df2862, 0x58c542c5, 0xdc71898d, 0x3248d382, 0xc13ad060, 0x52beac9f, 0xc449d892, 
  0x58fb0568, 0xdbcb0cce, 0x2f7afdfc, 0xc1bb5a11, 0x51d1dc80, 0xc3bdbdf6, 0x592d59da, 0xdb25f566, 
  0x2c9caf6c, 0xc2517e31, 0x50d86e6d, 0xc33aee27, 0x595c3e2a, 0xda8249b4, 0x29aee694, 0xc2fd08a9, 
  0x4fd288dc, 0xc2c17d52, 0x5987b08a, 0xd9e01006, 0x26b2a794, 0xc3bdbdf6, 0x4ec05432, 0xc2517e31, 
  0x59afaf4c, 0xd93f4e9e, 0x23a8fb93, 0xc4935b3c, 0x4da1fab5, 0xc1eb0209, 0x59d438e5, 0xd8a00bae, 
  0x2092f05f, 0xc57d965d, 0x4c77a88e, 0xc18e18a7, 0x59f54bee, 0xd8024d59, 0x1d719810, 0xc67c1e18, 
  0x4b418bbe, 0xc13ad060, 0x5a12e720, 0xd76619b6, 0x1a4608ab, 0xc78e9a1d, 0x49ffd417, 0xc0f1360b, 
  0x5a2d0957, 0xd6cb76c9, 0x17115bc0, 0xc8b4ab32, 0x48b2b335, 0xc0b15502, 0x5a43b190, 0xd6326a88, 
  0x13d4ae08, 0xc9edeb50, 0x475a5c77, 0xc07b371e, 0x5a56deec, 0xd59afadb, 0x10911f04, 0xcb39edca, 
  0x45f704f7, 0xc04ee4b8, 0x5a6690ae, 0xd5052d97, 0x0d47d096, 0xcc983f70, 0x4488e37f, 0xc02c64a6, 
  0x5a72c63b, 0xd4710883, 0x09f9e6a1, 0xce0866b8, 0x43103085, 0xc013bc39, 0x5a7b7f1a, 0xd3de9156, 
  0x06a886a0, 0xcf89e3e8, 0x418d2621, 0xc004ef3f, 0x5a80baf6, 0xd34dcdb4, 0x0354d741, 0xd11c3142, 
  0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 0x00000000, 0xd2bec333, 0x3e68fb62, 0xc004ef3f, 
  0x5a80baf6, 0xd2317756, 0xfcab28bf, 0xd4710883, 0x3cc85709, 0xc013bc39, 0x5a7b7f1a, 0xd1a5ef90, 
  0xf9577960, 0xd6326a88, 0x3b1e5335, 0xc02c64a6, 0x5a72c63b, 0xd11c3142, 0xf606195f, 0xd8024d59, 
  0x396b3199, 0xc04ee4b8, 0x5a6690ae, 0xd09441bb, 0xf2b82f6a, 0xd9e01006, 0x37af354c, 0xc07b371e, 
  0x5a56deec, 0xd00e2639, 0xef6ee0fc, 0xdbcb0cce, 0x35eaa2c7, 0xc0b15502, 0x5a43b190, 0xcf89e3e8, 
  0xec2b51f8, 0xddc29958, 0x341dbfd3, 0xc0f1360b, 0x5a2d0957, 0xcf077fe1, 0xe8eea440, 0xdfc606f1, 
  0x3248d382, 0xc13ad060, 0x5a12e720, 0xce86ff2a, 0xe5b9f755, 0xe1d4a2c8, 0x306c2624, 0xc18e18a7, 
  0x59f54bee, 0xce0866b8, 0xe28e67f0, 0xe3edb628, 0x2e88013a, 0xc1eb0209, 0x59d438e5, 0xcd8bbb6d, 
  0xdf6d0fa1, 0xe61086bc, 0x2c9caf6c, 0xc2517e31, 0x59afaf4c, 0xcd110216, 0xdc57046d, 0xe83c56cf, 
  0x2aaa7c7f, 0xc2c17d52, 0x5987b08a, 0xcc983f70, 0xd94d586c, 0xea70658a, 0x28b1b544, 0xc33aee27, 
  0x595c3e2a, 0xcc217822, 0xd651196c, 0xecabef3d, 0x26b2a794, 0xc3bdbdf6, 0x592d59da, 0xcbacb0bf, 
  0xd3635094, 0xeeee2d9d, 0x24ada23d, 0xc449d892, 0x58fb0568, 0xcb39edca, 0xd0850204, 0xf136580d, 
  0x22a2f4f8, 0xc4df2862, 0x58c542c5, 0xcac933ae, 0xcdb72c7e, 0xf383a3e2, 0x2092f05f, 0xc57d965d, 
  0x588c1404, 0xca5a86c4, 0xcafac90f, 0xf5d544a7, 0x1e7de5df, 0xc6250a18, 0x584f7b58, 0xc9edeb50, 
  0xc850cab4, 0xf82a6c6a, 0x1c6427a9, 0xc6d569be, 0x580f7b19, 0xc9836582, 0xc5ba1e09, 0xfa824bfd, 
  0x1a4608ab, 0xc78e9a1d, 0x57cc15bc, 0xc91af976, 0xc337a8f7, 0xfcdc1342, 0x1823dc7d, 0xc8507ea7, 
  0x57854ddd, 0xc8b4ab32, 0xc0ca4a63, 0xff36f170, 0x15fdf758, 0xc91af976, 0x573b2635, 0xc8507ea7, 
  0xbe72d9df, 0x0192155f, 0x13d4ae08, 0xc9edeb50, 0x56eda1a0, 0xc7ee77b3, 0xbc322766, 0x03ecadcf, 
  0x11a855df, 0xcac933ae, 0x569cc31b, 0xc78e9a1d, 0xba08fb09, 0x0645e9af, 0x0f7944a7, 0xcbacb0bf, 
  0x56488dc5, 0xc730e997, 0xb7f814b5, 0x089cf867, 0x0d47d096, 0xcc983f70, 0x55f104dc, 0xc6d569be, 
  0xb6002be9, 0x0af10a22, 0x0b145041, 0xcd8bbb6d, 0x55962bc0, 0xc67c1e18, 0xb421ef77, 0x0d415013, 
  0x08df1a8c, 0xce86ff2a, 0x553805f2, 0xc6250a18, 0xb25e054b, 0x0f8cfcbe, 0x06a886a0, 0xcf89e3e8, 
  0x54d69714, 0xc5d03118, 0xb0b50a2f, 0x11d3443f, 0x0470ebdc, 0xd09441bb, 0x5471e2e6, 0xc57d965d, 
  0xaf279193, 0x14135c94, 0x0238a1c6, 0xd1a5ef90, 0x5409ed4b, 0xc52d3d18, 0xadb6255e, 0x164c7ddd, 
  0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862, 0xac6145bb, 0x187de2a7, 0xfdc75e3a, 0xd3de9156, 
  0x53304df6, 0xc4935b3c, 0xab2968ec, 0x1aa6c82b, 0xfb8f1424, 0xd5052d97, 0x52beac9f, 0xc449d892, 
  0xaa0efb24, 0x1cc66e99, 0xf9577960, 0xd6326a88, 0x5249daa2, 0xc402a33c, 0xa9125e60, 0x1edc1953, 
  0xf720e574, 0xd76619b6, 0x51d1dc80, 0xc3bdbdf6, 0xa833ea44, 0x20e70f32, 0xf4ebafbf, 0xd8a00bae, 
  0x5156b6d9, 0xc37b2b6a, 0xa773ebfc, 0x22e69ac8, 0xf2b82f6a, 0xd9e01006, 0x50d86e6d, 0xc33aee27, 
  0xa6d2a626, 0x24da0a9a, 0xf086bb59, 0xdb25f566, 0x50570819, 0xc2fd08a9, 0xa65050b4, 0x26c0b162, 
  0xee57aa21, 0xdc71898d, 0x4fd288dc, 0xc2c17d52, 0xa5ed18e0, 0x2899e64a, 0xec2b51f8, 0xddc29958, 
  0x4f4af5d1, 0xc2884e6e, 0xa5a92114, 0x2a650525, 0xea0208a8, 0xdf18f0ce, 0x4ec05432, 0xc2517e31, 
  0xa58480e6, 0x2c216eaa, 0xe7dc2383, 0xe0745b24, 0x4e32a956, 0xc21d0eb8, 0xa57f450a, 0x2dce88aa, 
  0xe5b9f755, 0xe1d4a2c8, 0x4da1fab5, 0xc1eb0209, 0xa5996f52, 0x2f6bbe45, 0xe39bd857, 0xe3399167, 
  0x4d0e4de2, 0xc1bb5a11, 0xa5d2f6a9, 0x30f8801f, 0xe1821a21, 0xe4a2eff6, 0x4c77a88e, 0xc18e18a7, 
  0xa62bc71b, 0x32744493, 0xdf6d0fa1, 0xe61086bc, 0x4bde1089, 0xc1633f8a, 0xa6a3c1d6, 0x33de87de, 
  0xdd5d0b08, 0xe7821d59, 0x4b418bbe, 0xc13ad060, 0xa73abd3b, 0x3536cc52, 0xdb525dc3, 0xe8f77acf, 
  0x4aa22036, 0xc114ccb9, 0xa7f084e7, 0x367c9a7e, 0xd94d586c, 0xea70658a, 0x49ffd417, 0xc0f1360b, 
  0xa8c4d9cb, 0x37af8159, 0xd74e4abc, 0xebeca36c, 0x495aada2, 0xc0d00db6, 0xa9b7723b, 0x38cf1669, 
  0xd5558381, 0xed6bf9d1, 0x48b2b335, 0xc0b15502, 0xaac7fa0e, 0x39daf5e8, 0xd3635094, 0xeeee2d9d, 
  0x4807eb4b, 0xc0950d1d, 0xabf612b5, 0x3ad2c2e8, 0xd177fec6, 0xf0730342, 0x475a5c77, 0xc07b371e, 
  0xad415361, 0x3bb6276e, 0xcf93d9dc, 0xf1fa3ecb, 0x46aa0d6d, 0xc063d405, 0xaea94927, 0x3c84d496, 
  0xcdb72c7e, 0xf383a3e2, 0x45f704f7, 0xc04ee4b8, 0xb02d7724, 0x3d3e82ae, 0xcbe2402d, 0xf50ef5de, 
  0x454149fc, 0xc03c6a07, 0xb1cd56aa, 0x3de2f148, 0xca155d39, 0xf69bf7c9, 0x4488e37f, 0xc02c64a6, 
  0xb3885772, 0x3e71e759, 0xc850cab4, 0xf82a6c6a, 0x43cdd89a, 0xc01ed535, 0xb55ddfca, 0x3eeb3347, 
  0xc694ce67, 0xf9ba1651, 0x43103085, 0xc013bc39, 0xb74d4ccb, 0x3f4eaafe, 0xc4e1accb, 0xfb4ab7db, 
  0x424ff28f, 0xc00b1a20, 0xb955f293, 0x3f9c2bfb, 0xc337a8f7, 0xfcdc1342, 0x418d2621, 0xc004ef3f, 
  0xbb771c81, 0x3fd39b5a, 0xc197049e, 0xfe6deaa1, 0x40c7d2bd, 0xc0013bd3, 0xbdb00d71, 0x3ff4e5e0, 
};

static s32 AAC_TWID_TAB_EVEN[4*6 + 16*6 + 64*6] = {
  0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x5a82799a, 0xd2bec333, 
  0x539eba45, 0xe7821d59, 0x539eba45, 0xc4df2862, 0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 
  0x00000000, 0xd2bec333, 0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862, 0xac6145bb, 0x187de2a7, 
	
  0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x4b418bbe, 0xf383a3e2, 
  0x45f704f7, 0xf9ba1651, 0x4fd288dc, 0xed6bf9d1, 0x539eba45, 0xe7821d59, 0x4b418bbe, 0xf383a3e2, 
  0x58c542c5, 0xdc71898d, 0x58c542c5, 0xdc71898d, 0x4fd288dc, 0xed6bf9d1, 0x5a12e720, 0xce86ff2a, 
  0x5a82799a, 0xd2bec333, 0x539eba45, 0xe7821d59, 0x539eba45, 0xc4df2862, 0x58c542c5, 0xcac933ae, 
  0x569cc31b, 0xe1d4a2c8, 0x45f704f7, 0xc04ee4b8, 0x539eba45, 0xc4df2862, 0x58c542c5, 0xdc71898d, 
  0x3248d382, 0xc13ad060, 0x4b418bbe, 0xc13ad060, 0x5a12e720, 0xd76619b6, 0x1a4608ab, 0xc78e9a1d, 
  0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 0x00000000, 0xd2bec333, 0x3248d382, 0xc13ad060, 
  0x5a12e720, 0xce86ff2a, 0xe5b9f755, 0xe1d4a2c8, 0x22a2f4f8, 0xc4df2862, 0x58c542c5, 0xcac933ae, 
  0xcdb72c7e, 0xf383a3e2, 0x11a855df, 0xcac933ae, 0x569cc31b, 0xc78e9a1d, 0xba08fb09, 0x0645e9af, 
  0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862, 0xac6145bb, 0x187de2a7, 0xee57aa21, 0xdc71898d, 
  0x4fd288dc, 0xc2c17d52, 0xa5ed18e0, 0x2899e64a, 0xdd5d0b08, 0xe7821d59, 0x4b418bbe, 0xc13ad060, 
  0xa73abd3b, 0x3536cc52, 0xcdb72c7e, 0xf383a3e2, 0x45f704f7, 0xc04ee4b8, 0xb02d7724, 0x3d3e82ae, 
	
  0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x43103085, 0xfcdc1342, 
  0x418d2621, 0xfe6deaa1, 0x4488e37f, 0xfb4ab7db, 0x45f704f7, 0xf9ba1651, 0x43103085, 0xfcdc1342, 
  0x48b2b335, 0xf69bf7c9, 0x48b2b335, 0xf69bf7c9, 0x4488e37f, 0xfb4ab7db, 0x4c77a88e, 0xf1fa3ecb, 
  0x4b418bbe, 0xf383a3e2, 0x45f704f7, 0xf9ba1651, 0x4fd288dc, 0xed6bf9d1, 0x4da1fab5, 0xf0730342, 
  0x475a5c77, 0xf82a6c6a, 0x52beac9f, 0xe8f77acf, 0x4fd288dc, 0xed6bf9d1, 0x48b2b335, 0xf69bf7c9, 
  0x553805f2, 0xe4a2eff6, 0x51d1dc80, 0xea70658a, 0x49ffd417, 0xf50ef5de, 0x573b2635, 0xe0745b24, 
  0x539eba45, 0xe7821d59, 0x4b418bbe, 0xf383a3e2, 0x58c542c5, 0xdc71898d, 0x553805f2, 0xe4a2eff6, 
  0x4c77a88e, 0xf1fa3ecb, 0x59d438e5, 0xd8a00bae, 0x569cc31b, 0xe1d4a2c8, 0x4da1fab5, 0xf0730342, 
  0x5a6690ae, 0xd5052d97, 0x57cc15bc, 0xdf18f0ce, 0x4ec05432, 0xeeee2d9d, 0x5a7b7f1a, 0xd1a5ef90, 
  0x58c542c5, 0xdc71898d, 0x4fd288dc, 0xed6bf9d1, 0x5a12e720, 0xce86ff2a, 0x5987b08a, 0xd9e01006, 
  0x50d86e6d, 0xebeca36c, 0x592d59da, 0xcbacb0bf, 0x5a12e720, 0xd76619b6, 0x51d1dc80, 0xea70658a, 
  0x57cc15bc, 0xc91af976, 0x5a6690ae, 0xd5052d97, 0x52beac9f, 0xe8f77acf, 0x55f104dc, 0xc6d569be, 
  0x5a82799a, 0xd2bec333, 0x539eba45, 0xe7821d59, 0x539eba45, 0xc4df2862, 0x5a6690ae, 0xd09441bb, 
  0x5471e2e6, 0xe61086bc, 0x50d86e6d, 0xc33aee27, 0x5a12e720, 0xce86ff2a, 0x553805f2, 0xe4a2eff6, 
  0x4da1fab5, 0xc1eb0209, 0x5987b08a, 0xcc983f70, 0x55f104dc, 0xe3399167, 0x49ffd417, 0xc0f1360b, 
  0x58c542c5, 0xcac933ae, 0x569cc31b, 0xe1d4a2c8, 0x45f704f7, 0xc04ee4b8, 0x57cc15bc, 0xc91af976, 
  0x573b2635, 0xe0745b24, 0x418d2621, 0xc004ef3f, 0x569cc31b, 0xc78e9a1d, 0x57cc15bc, 0xdf18f0ce, 
  0x3cc85709, 0xc013bc39, 0x553805f2, 0xc6250a18, 0x584f7b58, 0xddc29958, 0x37af354c, 0xc07b371e, 
  0x539eba45, 0xc4df2862, 0x58c542c5, 0xdc71898d, 0x3248d382, 0xc13ad060, 0x51d1dc80, 0xc3bdbdf6, 
  0x592d59da, 0xdb25f566, 0x2c9caf6c, 0xc2517e31, 0x4fd288dc, 0xc2c17d52, 0x5987b08a, 0xd9e01006, 
  0x26b2a794, 0xc3bdbdf6, 0x4da1fab5, 0xc1eb0209, 0x59d438e5, 0xd8a00bae, 0x2092f05f, 0xc57d965d, 
  0x4b418bbe, 0xc13ad060, 0x5a12e720, 0xd76619b6, 0x1a4608ab, 0xc78e9a1d, 0x48b2b335, 0xc0b15502, 
  0x5a43b190, 0xd6326a88, 0x13d4ae08, 0xc9edeb50, 0x45f704f7, 0xc04ee4b8, 0x5a6690ae, 0xd5052d97, 
  0x0d47d096, 0xcc983f70, 0x43103085, 0xc013bc39, 0x5a7b7f1a, 0xd3de9156, 0x06a886a0, 0xcf89e3e8, 
  0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 0x00000000, 0xd2bec333, 0x3cc85709, 0xc013bc39, 
  0x5a7b7f1a, 0xd1a5ef90, 0xf9577960, 0xd6326a88, 0x396b3199, 0xc04ee4b8, 0x5a6690ae, 0xd09441bb, 
  0xf2b82f6a, 0xd9e01006, 0x35eaa2c7, 0xc0b15502, 0x5a43b190, 0xcf89e3e8, 0xec2b51f8, 0xddc29958, 
  0x3248d382, 0xc13ad060, 0x5a12e720, 0xce86ff2a, 0xe5b9f755, 0xe1d4a2c8, 0x2e88013a, 0xc1eb0209, 
  0x59d438e5, 0xcd8bbb6d, 0xdf6d0fa1, 0xe61086bc, 0x2aaa7c7f, 0xc2c17d52, 0x5987b08a, 0xcc983f70, 
  0xd94d586c, 0xea70658a, 0x26b2a794, 0xc3bdbdf6, 0x592d59da, 0xcbacb0bf, 0xd3635094, 0xeeee2d9d, 
  0x22a2f4f8, 0xc4df2862, 0x58c542c5, 0xcac933ae, 0xcdb72c7e, 0xf383a3e2, 0x1e7de5df, 0xc6250a18, 
  0x584f7b58, 0xc9edeb50, 0xc850cab4, 0xf82a6c6a, 0x1a4608ab, 0xc78e9a1d, 0x57cc15bc, 0xc91af976, 
  0xc337a8f7, 0xfcdc1342, 0x15fdf758, 0xc91af976, 0x573b2635, 0xc8507ea7, 0xbe72d9df, 0x0192155f, 
  0x11a855df, 0xcac933ae, 0x569cc31b, 0xc78e9a1d, 0xba08fb09, 0x0645e9af, 0x0d47d096, 0xcc983f70, 
  0x55f104dc, 0xc6d569be, 0xb6002be9, 0x0af10a22, 0x08df1a8c, 0xce86ff2a, 0x553805f2, 0xc6250a18, 
  0xb25e054b, 0x0f8cfcbe, 0x0470ebdc, 0xd09441bb, 0x5471e2e6, 0xc57d965d, 0xaf279193, 0x14135c94, 
  0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862, 0xac6145bb, 0x187de2a7, 0xfb8f1424, 0xd5052d97, 
  0x52beac9f, 0xc449d892, 0xaa0efb24, 0x1cc66e99, 0xf720e574, 0xd76619b6, 0x51d1dc80, 0xc3bdbdf6, 
  0xa833ea44, 0x20e70f32, 0xf2b82f6a, 0xd9e01006, 0x50d86e6d, 0xc33aee27, 0xa6d2a626, 0x24da0a9a, 
  0xee57aa21, 0xdc71898d, 0x4fd288dc, 0xc2c17d52, 0xa5ed18e0, 0x2899e64a, 0xea0208a8, 0xdf18f0ce, 
  0x4ec05432, 0xc2517e31, 0xa58480e6, 0x2c216eaa, 0xe5b9f755, 0xe1d4a2c8, 0x4da1fab5, 0xc1eb0209, 
  0xa5996f52, 0x2f6bbe45, 0xe1821a21, 0xe4a2eff6, 0x4c77a88e, 0xc18e18a7, 0xa62bc71b, 0x32744493, 
  0xdd5d0b08, 0xe7821d59, 0x4b418bbe, 0xc13ad060, 0xa73abd3b, 0x3536cc52, 0xd94d586c, 0xea70658a, 
  0x49ffd417, 0xc0f1360b, 0xa8c4d9cb, 0x37af8159, 0xd5558381, 0xed6bf9d1, 0x48b2b335, 0xc0b15502, 
  0xaac7fa0e, 0x39daf5e8, 0xd177fec6, 0xf0730342, 0x475a5c77, 0xc07b371e, 0xad415361, 0x3bb6276e, 
  0xcdb72c7e, 0xf383a3e2, 0x45f704f7, 0xc04ee4b8, 0xb02d7724, 0x3d3e82ae, 0xca155d39, 0xf69bf7c9, 
  0x4488e37f, 0xc02c64a6, 0xb3885772, 0x3e71e759, 0xc694ce67, 0xf9ba1651, 0x43103085, 0xc013bc39, 
  0xb74d4ccb, 0x3f4eaafe, 0xc337a8f7, 0xfcdc1342, 0x418d2621, 0xc004ef3f, 0xbb771c81, 0x3fd39b5a, 
};

AAC_DEF void aac_coefs_r4_core(s32 *coefs, s32 bg, s32 gp, s32 *wtab) {
  s32 ar, ai, br, bi, cr, ci, dr, di, tr, ti;
  s32 wd, ws, wi;
  s32 i, j, step;
  s32 *xptr, *wptr;

  for (; bg != 0; gp <<= 2, bg >>= 2) {

    step = 2*gp;
    xptr = coefs;

    for (i = bg; i != 0; i--) {

      wptr = wtab;

      for (j = gp; j != 0; j--) {

	ar = xptr[0];
	ai = xptr[1];
	xptr += step;
				
	ws = wptr[0];
	wi = wptr[1];
	br = xptr[0];
	bi = xptr[1];
	wd = ws + 2*wi;
	tr = aac_mulshift_32(wi, br + bi);
	br = aac_mulshift_32(wd, br) - tr;
	bi = aac_mulshift_32(ws, bi) + tr;
	xptr += step;
				
	ws = wptr[2];
	wi = wptr[3];
	cr = xptr[0];
	ci = xptr[1];
	wd = ws + 2*wi;
	tr = aac_mulshift_32(wi, cr + ci);
	cr = aac_mulshift_32(wd, cr) - tr;
	ci = aac_mulshift_32(ws, ci) + tr;
	xptr += step;
				
	ws = wptr[4];
	wi = wptr[5];
	dr = xptr[0];
	di = xptr[1];
	wd = ws + 2*wi;
	tr = aac_mulshift_32(wi, dr + di);
	dr = aac_mulshift_32(wd, dr) - tr;
	di = aac_mulshift_32(ws, di) + tr;
	wptr += 6;

	tr = ar;
	ti = ai;
	ar = (tr >> 2) - br;
	ai = (ti >> 2) - bi;
	br = (tr >> 2) + br;
	bi = (ti >> 2) + bi;

	tr = cr;
	ti = ci;
	cr = tr + dr;
	ci = di - ti;
	dr = tr - dr;
	di = di + ti;

	xptr[0] = ar + ci;
	xptr[1] = ai + dr;
	xptr -= step;
	xptr[0] = br - cr;
	xptr[1] = bi - di;
	xptr -= step;
	xptr[0] = ar - ci;
	xptr[1] = ai - dr;
	xptr -= step;
	xptr[0] = br + cr;
	xptr[1] = bi + di;
	xptr += 2;
      }
      xptr += 3*step;
    }
    wtab += 3*step;
  }  
}

AAC_DEF void aac_coefs_r4fft(s32 *coefs, s32 tab) {
  s32 order = AAC_NFFT_LOG_2_TAB[tab];
  s32 nfft = AAC_NFFT_TAB[tab];

  aac_coefs_bitreverse(coefs, tab);

  if(order & 0x1) {
    aac_coefs_r8_first_pass(coefs, nfft >> 3);
    aac_coefs_r4_core(coefs, nfft >> 5, 8, (s32 *) AAC_TWID_TAB_ODD);
  } else {
    aac_coefs_r4_first_pass(coefs, nfft >> 2);
    aac_coefs_r4_core(coefs, nfft >> 4, 4, (s32 *) AAC_TWID_TAB_EVEN);
  }
}

static s32 AAC_COS_1_SIN_1_TAB[514] = {
  /* format = Q30 */
  0x40000000, 0x00000000, 0x40323034, 0x003243f1, 0x406438cf, 0x006487c4, 0x409619b2, 0x0096cb58, 
  0x40c7d2bd, 0x00c90e90, 0x40f963d3, 0x00fb514b, 0x412accd4, 0x012d936c, 0x415c0da3, 0x015fd4d2, 
  0x418d2621, 0x0192155f, 0x41be162f, 0x01c454f5, 0x41eeddaf, 0x01f69373, 0x421f7c84, 0x0228d0bb, 
  0x424ff28f, 0x025b0caf, 0x42803fb2, 0x028d472e, 0x42b063d0, 0x02bf801a, 0x42e05ecb, 0x02f1b755, 
  0x43103085, 0x0323ecbe, 0x433fd8e1, 0x03562038, 0x436f57c1, 0x038851a2, 0x439ead09, 0x03ba80df, 
  0x43cdd89a, 0x03ecadcf, 0x43fcda59, 0x041ed854, 0x442bb227, 0x0451004d, 0x445a5fe8, 0x0483259d, 
  0x4488e37f, 0x04b54825, 0x44b73ccf, 0x04e767c5, 0x44e56bbd, 0x0519845e, 0x4513702a, 0x054b9dd3, 
  0x454149fc, 0x057db403, 0x456ef916, 0x05afc6d0, 0x459c7d5a, 0x05e1d61b, 0x45c9d6af, 0x0613e1c5, 
  0x45f704f7, 0x0645e9af, 0x46240816, 0x0677edbb, 0x4650dff1, 0x06a9edc9, 0x467d8c6d, 0x06dbe9bb, 
  0x46aa0d6d, 0x070de172, 0x46d662d6, 0x073fd4cf, 0x47028c8d, 0x0771c3b3, 0x472e8a76, 0x07a3adff, 
  0x475a5c77, 0x07d59396, 0x47860275, 0x08077457, 0x47b17c54, 0x08395024, 0x47dcc9f9, 0x086b26de, 
  0x4807eb4b, 0x089cf867, 0x4832e02d, 0x08cec4a0, 0x485da887, 0x09008b6a, 0x4888443d, 0x09324ca7, 
  0x48b2b335, 0x09640837, 0x48dcf556, 0x0995bdfd, 0x49070a84, 0x09c76dd8, 0x4930f2a6, 0x09f917ac, 
  0x495aada2, 0x0a2abb59, 0x49843b5f, 0x0a5c58c0, 0x49ad9bc2, 0x0a8defc3, 0x49d6ceb3, 0x0abf8043, 
  0x49ffd417, 0x0af10a22, 0x4a28abd6, 0x0b228d42, 0x4a5155d6, 0x0b540982, 0x4a79d1ff, 0x0b857ec7, 
  0x4aa22036, 0x0bb6ecef, 0x4aca4065, 0x0be853de, 0x4af23270, 0x0c19b374, 0x4b19f641, 0x0c4b0b94, 
  0x4b418bbe, 0x0c7c5c1e, 0x4b68f2cf, 0x0cada4f5, 0x4b902b5c, 0x0cdee5f9, 0x4bb7354d, 0x0d101f0e, 
  0x4bde1089, 0x0d415013, 0x4c04bcf8, 0x0d7278eb, 0x4c2b3a84, 0x0da39978, 0x4c518913, 0x0dd4b19a, 
  0x4c77a88e, 0x0e05c135, 0x4c9d98de, 0x0e36c82a, 0x4cc359ec, 0x0e67c65a, 0x4ce8eb9f, 0x0e98bba7, 
  0x4d0e4de2, 0x0ec9a7f3, 0x4d33809c, 0x0efa8b20, 0x4d5883b7, 0x0f2b650f, 0x4d7d571c, 0x0f5c35a3, 
  0x4da1fab5, 0x0f8cfcbe, 0x4dc66e6a, 0x0fbdba40, 0x4deab226, 0x0fee6e0d, 0x4e0ec5d1, 0x101f1807, 
  0x4e32a956, 0x104fb80e, 0x4e565c9f, 0x10804e06, 0x4e79df95, 0x10b0d9d0, 0x4e9d3222, 0x10e15b4e, 
  0x4ec05432, 0x1111d263, 0x4ee345ad, 0x11423ef0, 0x4f06067f, 0x1172a0d7, 0x4f289692, 0x11a2f7fc, 
  0x4f4af5d1, 0x11d3443f, 0x4f6d2427, 0x12038584, 0x4f8f217e, 0x1233bbac, 0x4fb0edc1, 0x1263e699, 
  0x4fd288dc, 0x1294062f, 0x4ff3f2bb, 0x12c41a4f, 0x50152b47, 0x12f422db, 0x5036326e, 0x13241fb6, 
  0x50570819, 0x135410c3, 0x5077ac37, 0x1383f5e3, 0x50981eb1, 0x13b3cefa, 0x50b85f74, 0x13e39be9, 
  0x50d86e6d, 0x14135c94, 0x50f84b87, 0x144310dd, 0x5117f6ae, 0x1472b8a5, 0x51376fd0, 0x14a253d1, 
  0x5156b6d9, 0x14d1e242, 0x5175cbb5, 0x150163dc, 0x5194ae52, 0x1530d881, 0x51b35e9b, 0x15604013, 
  0x51d1dc80, 0x158f9a76, 0x51f027eb, 0x15bee78c, 0x520e40cc, 0x15ee2738, 0x522c270f, 0x161d595d, 
  0x5249daa2, 0x164c7ddd, 0x52675b72, 0x167b949d, 0x5284a96e, 0x16aa9d7e, 0x52a1c482, 0x16d99864, 
  0x52beac9f, 0x17088531, 0x52db61b0, 0x173763c9, 0x52f7e3a6, 0x1766340f, 0x5314326d, 0x1794f5e6, 
  0x53304df6, 0x17c3a931, 0x534c362d, 0x17f24dd3, 0x5367eb03, 0x1820e3b0, 0x53836c66, 0x184f6aab, 
  0x539eba45, 0x187de2a7, 0x53b9d48f, 0x18ac4b87, 0x53d4bb34, 0x18daa52f, 0x53ef6e23, 0x1908ef82, 
  0x5409ed4b, 0x19372a64, 0x5424389d, 0x196555b8, 0x543e5007, 0x19937161, 0x5458337a, 0x19c17d44, 
  0x5471e2e6, 0x19ef7944, 0x548b5e3b, 0x1a1d6544, 0x54a4a56a, 0x1a4b4128, 0x54bdb862, 0x1a790cd4, 
  0x54d69714, 0x1aa6c82b, 0x54ef4171, 0x1ad47312, 0x5507b76a, 0x1b020d6c, 0x551ff8ef, 0x1b2f971e, 
  0x553805f2, 0x1b5d100a, 0x554fde64, 0x1b8a7815, 0x55678236, 0x1bb7cf23, 0x557ef15a, 0x1be51518, 
  0x55962bc0, 0x1c1249d8, 0x55ad315b, 0x1c3f6d47, 0x55c4021d, 0x1c6c7f4a, 0x55da9df7, 0x1c997fc4, 
  0x55f104dc, 0x1cc66e99, 0x560736bd, 0x1cf34baf, 0x561d338d, 0x1d2016e9, 0x5632fb3f, 0x1d4cd02c, 
  0x56488dc5, 0x1d79775c, 0x565deb11, 0x1da60c5d, 0x56731317, 0x1dd28f15, 0x568805c9, 0x1dfeff67, 
  0x569cc31b, 0x1e2b5d38, 0x56b14b00, 0x1e57a86d, 0x56c59d6a, 0x1e83e0eb, 0x56d9ba4e, 0x1eb00696, 
  0x56eda1a0, 0x1edc1953, 0x57015352, 0x1f081907, 0x5714cf59, 0x1f340596, 0x572815a8, 0x1f5fdee6, 
  0x573b2635, 0x1f8ba4dc, 0x574e00f2, 0x1fb7575c, 0x5760a5d5, 0x1fe2f64c, 0x577314d2, 0x200e8190, 
  0x57854ddd, 0x2039f90f, 0x579750ec, 0x20655cac, 0x57a91df2, 0x2090ac4d, 0x57bab4e6, 0x20bbe7d8, 
  0x57cc15bc, 0x20e70f32, 0x57dd406a, 0x21122240, 0x57ee34e5, 0x213d20e8, 0x57fef323, 0x21680b0f, 
  0x580f7b19, 0x2192e09b, 0x581fccbc, 0x21bda171, 0x582fe804, 0x21e84d76, 0x583fcce6, 0x2212e492, 
  0x584f7b58, 0x223d66a8, 0x585ef351, 0x2267d3a0, 0x586e34c7, 0x22922b5e, 0x587d3fb0, 0x22bc6dca, 
  0x588c1404, 0x22e69ac8, 0x589ab1b9, 0x2310b23e, 0x58a918c6, 0x233ab414, 0x58b74923, 0x2364a02e, 
  0x58c542c5, 0x238e7673, 0x58d305a6, 0x23b836ca, 0x58e091bd, 0x23e1e117, 0x58ede700, 0x240b7543, 
  0x58fb0568, 0x2434f332, 0x5907eced, 0x245e5acc, 0x59149d87, 0x2487abf7, 0x5921172e, 0x24b0e699, 
  0x592d59da, 0x24da0a9a, 0x59396584, 0x250317df, 0x59453a24, 0x252c0e4f, 0x5950d7b3, 0x2554edd1, 
  0x595c3e2a, 0x257db64c, 0x59676d82, 0x25a667a7, 0x597265b4, 0x25cf01c8, 0x597d26b8, 0x25f78497, 
  0x5987b08a, 0x261feffa, 0x59920321, 0x264843d9, 0x599c1e78, 0x2670801a, 0x59a60288, 0x2698a4a6, 
  0x59afaf4c, 0x26c0b162, 0x59b924bc, 0x26e8a637, 0x59c262d5, 0x2710830c, 0x59cb698f, 0x273847c8, 
  0x59d438e5, 0x275ff452, 0x59dcd0d3, 0x27878893, 0x59e53151, 0x27af0472, 0x59ed5a5c, 0x27d667d5, 
  0x59f54bee, 0x27fdb2a7, 0x59fd0603, 0x2824e4cc, 0x5a048895, 0x284bfe2f, 0x5a0bd3a1, 0x2872feb6, 
  0x5a12e720, 0x2899e64a, 0x5a19c310, 0x28c0b4d2, 0x5a20676c, 0x28e76a37, 0x5a26d42f, 0x290e0661, 
  0x5a2d0957, 0x29348937, 0x5a3306de, 0x295af2a3, 0x5a38ccc2, 0x2981428c, 0x5a3e5afe, 0x29a778db, 
  0x5a43b190, 0x29cd9578, 0x5a48d074, 0x29f3984c, 0x5a4db7a6, 0x2a19813f, 0x5a526725, 0x2a3f503a, 
  0x5a56deec, 0x2a650525, 0x5a5b1efa, 0x2a8a9fea, 0x5a5f274b, 0x2ab02071, 0x5a62f7dd, 0x2ad586a3, 
  0x5a6690ae, 0x2afad269, 0x5a69f1bb, 0x2b2003ac, 0x5a6d1b03, 0x2b451a55, 0x5a700c84, 0x2b6a164d, 
  0x5a72c63b, 0x2b8ef77d, 0x5a754827, 0x2bb3bdce, 0x5a779246, 0x2bd8692b, 0x5a79a498, 0x2bfcf97c, 
  0x5a7b7f1a, 0x2c216eaa, 0x5a7d21cc, 0x2c45c8a0, 0x5a7e8cac, 0x2c6a0746, 0x5a7fbfbb, 0x2c8e2a87, 
  0x5a80baf6, 0x2cb2324c, 0x5a817e5d, 0x2cd61e7f, 0x5a8209f1, 0x2cf9ef09, 0x5a825db0, 0x2d1da3d5, 
  0x5a82799a, 0x2d413ccd, 
};

static s32 AAC_POST_SKIP[AAC_NUM_IMDCT_SIZES] = {15, 1};

AAC_DEF void aac_coefs_post_multiply_rescale(s32 *coefs, s32 es, s32 tab) {

  s32 nmdct = AAC_NMDCT_TAB[tab];
  s32 *csptr = AAC_COS_1_SIN_1_TAB;
  s32 skip_factor = AAC_POST_SKIP[tab];
  s32 *fft1 = coefs;
  s32 *fft2 = fft1 + nmdct - 1;

  s32 cs2 = *csptr++;
  s32 sin2 = *csptr;
  csptr += skip_factor;

  for(s32 i=nmdct>>2;i!=0;i--) {
    s32 ar1 = *(fft1 + 0);
    s32 ai1 = *(fft1 + 1);
    s32 ai2 = *(fft2 + 0);

    s32 t, z;

    t = aac_mulshift_32(sin2, ar1 + ai1);
    z = t - aac_mulshift_32(cs2, ai1);
    aac_clip_2n_shift(z, es);
    *fft2-- = z;
    cs2 = cs2 - 2*sin2;
    z = t + aac_mulshift_32(cs2, ar1);
    aac_clip_2n_shift(z, es);
    *fft1++ = z;

    cs2 = *csptr++;
    sin2 = *csptr;
    csptr += skip_factor;

    s32 ar2 = *fft2;
    ai2 = -ai2;
    t = aac_mulshift_32(sin2, ar2 + ai2);
    z = t - aac_mulshift_32(cs2, ai2);
    aac_clip_2n_shift(z, es);
    *fft2-- = z;
    cs2 = cs2 - 2*sin2;
    z = t + aac_mulshift_32(cs2, ar2);
    aac_clip_2n_shift(z, es);
    *fft1++ = z;
    cs2 = cs2 + 2*sin2;
  }
  
}

AAC_DEF void aac_coefs_pre_multiply(s32 *zbuf1, s32 tab) {

  s32 nmdct = AAC_NMDCT_TAB[tab];
  s32 *zbuf2 = zbuf1 + nmdct - 1;
  s32 *csptr = AAC_COS_4_SIN_4_TAB + AAC_COS_4_SIN_4_TAB_OFFSET[tab];

  for(s32 i=nmdct>>2;i!=0;i--) {

    s32 cps2a = *csptr++;
    s32 sin2a = *csptr++;
    s32 cps2b = *csptr++;
    s32 sin2b = *csptr++;

    s32 ar1 = *(zbuf1 + 0);
    s32 ai2 = *(zbuf1 + 1);
    s32 ai1 = *(zbuf2 + 0);
    s32 ar2 = *(zbuf2 - 1);

    s32 t, z1, z2, cms2;
    
    t = aac_mulshift_32(sin2a, ar1 + ai1);
    z2 = aac_mulshift_32(cps2a, ai1) - t;
    cms2 = cps2a - 2*sin2a;
    z1 = aac_mulshift_32(cms2, ar1) + t;
    *zbuf1++ = z1;
    *zbuf1++ = z2;

    t = aac_mulshift_32(sin2b, ar2 + ai2);
    z2 = aac_mulshift_32(cps2b, ai2) - t;
    cms2 = cps2b - 2*sin2b;
    z1 = aac_mulshift_32(cms2, ar2) + t;
    *zbuf2-- = z2;
    *zbuf2-- = z1;
  }
  
}

AAC_DEF void aac_coefs_post_multiply(s32 *fft1, s32 tab) {

  s32 nmdct = AAC_NMDCT_TAB[tab];
  s32 *csptr = AAC_COS_1_SIN_1_TAB;
  s32 skip_factor = AAC_POST_SKIP[tab];
  s32 *fft2 = fft1 + nmdct - 1;

  s32 cps2 = *csptr++;
  s32 sin2 = *csptr;
  csptr += skip_factor;
  s32 cms2 = cps2 - 2*sin2;

  for(s32 i=nmdct>>2;i!=0;i--) {
    s32 ar1 = *(fft1 + 0);
    s32 ai1 = *(fft1 + 1);
    s32 ar2 = *(fft2 - 1);
    s32 ai2 = *(fft2 + 0);

    s32 t;

    t = aac_mulshift_32(sin2, ar1 + ai1);
    *fft2-- = t - aac_mulshift_32(cps2, ai1);
    *fft1++ = t + aac_mulshift_32(cms2, ar1);
    
    cps2 = *csptr++;
    sin2 = *csptr;
    csptr += skip_factor;
    
    ai2 = -ai2;
    t = aac_mulshift_32(sin2, ar2 + ai2);
    *fft2-- = t - aac_mulshift_32(cps2, ai2);
    cms2 = cps2 - 2*sin2;
    *fft1++ = t + aac_mulshift_32(cms2, ar2);
  }
  
}

AAC_DEF void aac_coefs_generate_noise_vector(s32 *coefs,
					     s32 *last,
					     s32 width) {

  for(s32 i=0;i<width;i++) {
    coefs[i] = ((s32) aac_rand_u32((u32 *) last)) >> 16;
  }
  
}

AAC_DEF void aac_coefs_copy_noise_vector(s32 *coefs_left,
					 s32 *coefs_right,
					 s32 width) {
  for(s32 i=0;i<width;i++) {
    coefs_right[i] = coefs_left[i];
  }
}

AAC_DEF s32 aac_coefs_scale_noise_vector(s32 *coefs,
					 s32 width,
					 s32 sf) {

  static s32 POW_14[4] = { 
    0x40000000, 0x4c1bf829, 0x5a82799a, 0x6ba27e65
  };

  s32 energy = 0;
  for(s32 i=0;i<width;i++) {
    s32 spec = coefs[i];

    s32 sq = (spec *spec) >> 8;
    energy += sq;
  }

  if(energy == 0) {
    return 0;
  }

  s32 scale_f = POW_14[sf & 0x03];
  s32 scale_i = (sf >> 2) + AAC_FBITS_OUT_DQ_OFF;

  s32 z = aac_clz(energy) - 2;
  z = z & 0xfffffffe;

  s32 inverse_square_root_energy = aac_inverse_square_root(energy << z);
  scale_i -= (15 - z/2 + 4);

  z = aac_clz(inverse_square_root_energy) - 1;
  inverse_square_root_energy = inverse_square_root_energy << z;
  scale_i -= (z - 3 - 2);
  scale_f = aac_mulshift_32(scale_f, inverse_square_root_energy);

  s32 gb_mask = 0;
  if(scale_i < 0) {
    scale_i = -scale_i;
    if(scale_i > 31) {
      scale_i = 31;
    }

    for(s32 i=0;i<width;i++) {
      s32 c = aac_mulshift_32(coefs[i], scale_f) >> scale_i;
      gb_mask = gb_mask | aac_fastabs(c);
      coefs[i] = c;
    }
    
  } else {
    if(scale_i > 16) {
      scale_i = 16;
    }
    for(s32 i=0;i<width;i++) {
      s32 c = aac_mulshift_32(coefs[i] << scale_i, scale_f);
      gb_mask |= aac_fastabs(c);
      coefs[i] = c;
    }
    
  }
  
  return gb_mask;

}

AAC_DEF s32 aac_coefs_filter_region(s32 *coefs,
				    s32 *tns_lpc_buf,
				    s32 *tns_work_buf,
				    s16 size,
				    u8 dir,
				    u8 order) {

  for(u8 i=0;i<order;i++) {
    tns_work_buf[i] = 0;
  }

  Aac_Word sum64;
  sum64.w64 = 0;

  s32 gb_mask = 0;
  s32 inc;
  if(dir) {
    inc = -1;
  } else {
    inc = 1;
  }

  do {
    s32 y = *coefs;

    sum64.r.hi32 = y >> (32 - AAC_FBITS_LPC_COEFS);
    sum64.r.lo32 = y << AAC_FBITS_LPC_COEFS;

    for(u8 j=order-1;j>0;j--) {
      sum64.w64 = aac_madd_64(sum64.w64, tns_work_buf[j], tns_lpc_buf[j]);
      tns_work_buf[j] = tns_work_buf[j - 1];
    }

    sum64.w64 = aac_madd_64(sum64.w64, tns_work_buf[0], tns_lpc_buf[0]);
    y =
      (sum64.r.hi32 << (32 - AAC_FBITS_LPC_COEFS)) |
      (sum64.r.lo32 >> AAC_FBITS_LPC_COEFS);

    s32 hi32 = sum64.r.hi32;
    if((hi32 >> 31) != (hi32 >> (AAC_FBITS_LPC_COEFS - 1))) {
      y = (hi32 >> 31) ^ 0x7fffffff;
    }

    tns_work_buf[0] = y;
    *coefs = y;
    coefs += inc;
    gb_mask = gb_mask | aac_fastabs(y);
    
  } while(--size);

  return gb_mask;
}

AAC_DEF Aac_Error aac_decode_next_element(Aac *a, Aac_Bits *b) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;

  a->prev_block_id = a->curr_block_id;
  aac_unwrap(aac_bits_next_s32(b, AAC_NUM_SYN_ID_BITS, &a->curr_block_id));

  ps->common_window = 0;

  switch(a->curr_block_id) {

  case ___AAC_ID_SCE: { // 0
    aac_unwrap(aac_decode_single_channel_element(a, b));
  } break;

  case ___AAC_ID_CPE: { // 1
    aac_unwrap(aac_decode_channel_pair_element(a, b));
  } break;

  case ___AAC_ID_LFE: { // 3
    aac_unwrap(aac_decode_lfe_channel_element(a, b));
  } break;

  case ___AAC_ID_PCE : { // 5
    aac_unwrap(aac_decode_program_config_element(ps->pce + 0, b));
  } break;

  case ___AAC_ID_FIL: { // 6
    aac_unwrap(aac_decode_fill_element(a, b));
  } break;

  case ___AAC_ID_END: { // 7
  } break;
       
  default:
    aac_panic("handle: %d\n", a->curr_block_id);
    break;
  }

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_fill_element(Aac *a, Aac_Bits *b) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;

  u32 fill_count;
  aac_unwrap(aac_bits_next_u32(b, 4, &fill_count));

  if(fill_count == 15) {
    aac_unwrap(aac_bits_next_u32(b, 8, &fill_count));
    fill_count = 15 + (fill_count - 1);
  }

  ps->fill_count = fill_count;
  for(u32 i=0;i<fill_count;i++) {
    aac_unwrap(aac_bits_next_u8(b, 8, &ps->fill_buf[i]));
  }

  a->curr_inst_tag = -1;
  a->fill_ext_type =  0;

  a->fill_buf = ps->fill_buf;
  a->fill_count = ps->fill_count;
  
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_channel_pair_element(Aac *a, Aac_Bits *b) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;
  Aac_Ics_Info *ics = a->ps_info_base.ics_info;

  aac_unwrap(aac_bits_next_s32(b, AAC_NUM_INST_TAG_BITS, &a->curr_inst_tag));
  aac_unwrap(aac_bits_next_s32(b, 1, &ps->common_window));

  if(ps->common_window) {
    aac_unwrap(aac_decode_ics_info(ics, b, ps->sampling_rate_index));
    aac_unwrap(aac_bits_next_s32(b, 2, &ps->ms_mask_present));
    
    if(ps->ms_mask_present == 1) {
      u8 *mask_ptr = ps->ms_mask_bits;
      *mask_ptr = 0;
      s32 mask_offset = 0;
      for(s32 gp=0;gp<ics->num_window_group;gp++) {
	for(s32 sfb=0;sfb<ics->max_sfb;sfb++) { // Dont you need to `min` against AAC_MAX_SCALEFACTOR_SFB_TABLE? 
	  u8 curr_bit;
	  aac_unwrap(aac_bits_next_u8(b, 1, &curr_bit));
	  *mask_ptr |= curr_bit << mask_offset;
	  if(++mask_offset == 8) {
	    mask_ptr += 1;
	    *mask_ptr = 0;
	    mask_offset = 0;
	  }
	  
	}
      }
      
    }
  }
  
  return AAC_ERROR_NONE;  
}

AAC_DEF Aac_Error aac_decode_program_config_element(Aac_Prog_Config_Element *p, Aac_Bits *b) {
  
  aac_unwrap(aac_bits_next_u8(b, 4, &p->elem_inst_tag));
  aac_unwrap(aac_bits_next_u8(b, 2, &p->profile));
  aac_unwrap(aac_bits_next_u8(b, 4, &p->sampling_rate_index));
  aac_unwrap(aac_bits_next_u8(b, 4, &p->num_fce));
  aac_unwrap(aac_bits_next_u8(b, 4, &p->num_sce));
  aac_unwrap(aac_bits_next_u8(b, 4, &p->num_bce));
  aac_unwrap(aac_bits_next_u8(b, 2, &p->num_lce));
  aac_unwrap(aac_bits_next_u8(b, 3, &p->num_ade));
  aac_unwrap(aac_bits_next_u8(b, 4, &p->num_cce));

  u8 mono_mixdown;
  aac_unwrap(aac_bits_next_u8(b, 1, &mono_mixdown));
  if(mono_mixdown) {
    aac_unwrap(aac_bits_next_u8(b, 4, &p->mono_mixdown));
    p->mono_mixdown |= (mono_mixdown << 4);
  }

  u8 stereo_mixdown;
  aac_unwrap(aac_bits_next_u8(b, 1, &stereo_mixdown));
  if(stereo_mixdown) {
    aac_unwrap(aac_bits_next_u8(b, 4, &p->stereo_mixdown));
    p->stereo_mixdown |= (stereo_mixdown << 4);
  }

  u8 matrix_mixdown;
  aac_unwrap(aac_bits_next_u8(b, 1, &matrix_mixdown));
  if(matrix_mixdown) {
    aac_unwrap(aac_bits_next_u8(b, 2, &p->matrix_mixdown));
    matrix_mixdown = (matrix_mixdown << 2) | p->matrix_mixdown;

    aac_unwrap(aac_bits_next_u8(b, 1, &p->matrix_mixdown));
    p->matrix_mixdown |= (matrix_mixdown << 1);
  }

  u8 t;
  for(u8 i=0;i<p->num_fce;i++) {
    aac_unwrap(aac_bits_next_u8(b, 1, &t));
    aac_unwrap(aac_bits_next_u8(b, 4, &p->fce[i]));
    p->fce[i] |= t << 4;
  }

  for(u8 i=0;i<p->num_sce;i++) {
    aac_unwrap(aac_bits_next_u8(b, 1, &t));
    aac_unwrap(aac_bits_next_u8(b, 4, &p->sce[i]));
    p->sce[i] |= t << 4;
  }

  for(u8 i=0;i<p->num_bce;i++) {
    aac_unwrap(aac_bits_next_u8(b, 1, &t));
    aac_unwrap(aac_bits_next_u8(b, 4, &p->bce[i]));
    p->bce[i] |= t << 4;
  }

  for(u8 i=0;i<p->num_lce;i++) {
    aac_unwrap(aac_bits_next_u8(b, 4, &p->lce[i]));
  }

  for(u8 i=0;i<p->num_ade;i++) {
    aac_unwrap(aac_bits_next_u8(b, 4, &p->lce[i]));
  }

  for(u8 i=0;i<p->num_cce;i++) {
    aac_unwrap(aac_bits_next_u8(b, 1, &t));
    aac_unwrap(aac_bits_next_u8(b, 4, &p->cce[i]));
    p->cce[i] |= t << 4;
  }

  if(!aac_bits_are_byte_aligned(*b)) {
    // TODO: Verifiy this
    b->i = -1;
  }  

  // Discard comments
  s32 n;
  aac_unwrap(aac_bits_next_s32(b, 8, &n));
  aac_unwrap(aac_bits_discard(b, n * 8));

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_single_channel_element(Aac *a, Aac_Bits *b) {
  aac_unwrap(aac_bits_next_s32(b, AAC_NUM_INST_TAG_BITS, &a->curr_inst_tag));
  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_lfe_channel_element(Aac *a, Aac_Bits *b) {
  aac_unwrap(aac_bits_next_s32(b, AAC_NUM_INST_TAG_BITS, &a->curr_inst_tag));
  return AAC_ERROR_NONE;
}

static s32 AAC_CHANNEL_MAP_TABLE[] = {
  -1, 1, 2, 3, 4, 5, 6, 8,
};

AAC_DEF Aac_Error aac_decode_adts(Aac *a, Aac_Bits *b) {

  Aac_Ps_Info_Base *ps = &a->ps_info_base;
  Aac_Adts_Header *adts = &a->ps_info_base.adts;

  static u16 adts_id_expected = 0xfff;
  u16 adts_id;
  if(!aac_bits_next_u16(b, 12, &adts_id) ||
     adts_id != adts_id_expected) {
    return AAC_ERROR_UNREACHABLE;
  }

  aac_unwrap(aac_bits_next_u8(b, 1, &adts->id));
  aac_unwrap(aac_bits_next_u8(b, 2, &adts->layer));
  aac_unwrap(aac_bits_next_u8(b, 1, &adts->protect_bit));
  aac_unwrap(aac_bits_next_u8(b, 2, &adts->profile));
  aac_unwrap(aac_bits_next_u8(b, 4, &adts->sampling_rate_index));
  aac_unwrap(aac_bits_next_u8(b, 1, &adts->private_bit));
  aac_unwrap(aac_bits_next_u8(b, 3, &adts->channel_config));
  aac_unwrap(aac_bits_next_u8(b, 1, &adts->original_copy));
  aac_unwrap(aac_bits_next_u8(b, 1, &adts->home));

  aac_unwrap(aac_bits_next_u8(b, 1, &adts->copy_bit));
  aac_unwrap(aac_bits_next_u8(b, 1, &adts->copy_start));
  aac_unwrap(aac_bits_next_s32(b, 13, &adts->frame_length));
  aac_unwrap(aac_bits_next_s32(b, 11, &adts->buffer_full));
  aac_unwrap(aac_bits_next_u8(b, 2, &adts->num_raw_data_blocks));
  adts->num_raw_data_blocks += 1;

  if(adts->protect_bit == 0) {
    aac_unwrap(aac_bits_next_s32(b, 16, &adts->crc_check_word));
  }

  if(!aac_bits_are_byte_aligned(*b)) {
    return AAC_ERROR_UNREACHABLE;
  }

  ps->sampling_rate_index = adts->sampling_rate_index;  
  if(!ps->use_imp_channel_map) {    
    if(AAC_ARRAY_LEN(AAC_CHANNEL_MAP_TABLE) <= adts->channel_config) {
      return AAC_ERROR_UNKNOWN_CHANNEL_CONFIGURATION;
    }
    
    ps->channels = AAC_CHANNEL_MAP_TABLE[adts->channel_config];
  }

  a->prev_block_id = ___AAC_ID_INVALID;
  a->curr_block_id = ___AAC_ID_INVALID;
  a->curr_inst_tag = -1;

  a->bit_rate = 0;
  a->channels = ps->channels;
  if(AAC_ARRAY_LEN(AAC_SAMPLE_RATE_MAP_TABLE) <= adts->sampling_rate_index) {
    return AAC_ERROR_UNKNOWN_SAMPLING_FREQUENCY_INDEX;
  }
  a->sample_rate = AAC_SAMPLE_RATE_MAP_TABLE[adts->sampling_rate_index];
  a->profile = adts->profile;
  a->sbr_enabled = 0;
  a->adts_blocks_left = adts->num_raw_data_blocks;  

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_align_to_adts_sync_word(Aac_Bits *b) {

  if(!aac_bits_are_byte_aligned(*b)) {
    return AAC_ERROR_UNREACHABLE;
  }

  static u8 adts_id_hi = 0xff;
  static u8 adts_id_lo = 0xf0;
  
  for(u64 i=0;i<b->len - 1;i++) {
    if(b->data[i] == adts_id_hi &&
       (b->data[i + 1] & adts_id_lo) == adts_id_lo) {
      b->data += i;
      b->len  -= i;
      return AAC_ERROR_NONE;
    }
  }

  // If it is not present, request more data
  return AAC_ERROR_NOT_ENOUGH_DATA;
}

AAC_DEF Aac_Error aac_calculate_implicit_adts_channel_mapping(Aac *a, Aac_Bits *b) {

  s32 channels = 0;
  do {

    aac_unwrap(aac_decode_next_element(a, b));
    s32 element_channels = AAC_ELEMENT_NUM_CHANNELS_TABLE[a->curr_block_id];
    channels += element_channels;

    for(s32 channel=0;channel<element_channels;channel++) {
      aac_unwrap(aac_decode_noiseless_data(a, b, channel));
    }
    
  } while(a->curr_block_id != ___AAC_ID_END);
  if(channels <= 0) {
    return AAC_ERROR_UNREACHABLE;
  }
  a->ps_info_base.channels = channels;
  a->channels = a->ps_info_base.channels;
  a->ps_info_base.use_imp_channel_map = 1;

  return AAC_ERROR_NONE;
}

AAC_DEF Aac_Error aac_decode_adif(Aac *a, Aac_Bits *b) {
  (void) a;
  (void) b;
  AAC_TODO();
}

static s32 AAC_MAX_SCALEFACTOR_SFB_TABLE[AAC_NUM_SAMPLE_RATES] = {
  33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34,
};

AAC_DEF Aac_Error aac_decode_ics_info(Aac_Ics_Info *ics, Aac_Bits *b, s32 sampling_rate_index) {

  aac_unwrap(aac_bits_next_u8(b, 1, &ics->ics_res_bit));
  aac_unwrap(aac_bits_next_u8(b, 2, &ics->window_seq));
  aac_unwrap(aac_bits_next_u8(b, 1, &ics->window_shape));
  if(ics->window_seq == 2) {
    // short block
    aac_unwrap(aac_bits_next_u8(b, 4, &ics->max_sfb));
    aac_unwrap(aac_bits_next_u8(b, 7, &ics->sf_group));
    ics->num_window_group = 1;
    ics->window_group_len[0] = 1;

    s32 mask = 0x40;
    for(s32 g=0;g<7;g++) {
      if(ics->sf_group & mask) {
	ics->window_group_len[ics->num_window_group - 1] += 1;
      } else {
	ics->num_window_group += 1;
	ics->window_group_len[ics->num_window_group - 1] = 1;
      }
      mask = mask >> 1;
    }
    
  } else {
    // long block
    aac_unwrap(aac_bits_next_u8(b, 6, &ics->max_sfb));
    aac_unwrap(aac_bits_next_u8(b, 1, &ics->predictor_data_present));
    if(ics->predictor_data_present) {
      aac_unwrap(aac_bits_next_u8(b, 1, &ics->predictor_reset));
      if(ics->predictor_reset) {
	aac_unwrap(aac_bits_next_u8(b, 5, &ics->predictor_reset_group_num));
      }
      
      s32 sfb_max = ics->max_sfb;
      if(sfb_max < AAC_MAX_SCALEFACTOR_SFB_TABLE[sampling_rate_index]) {
	sfb_max = AAC_MAX_SCALEFACTOR_SFB_TABLE[sampling_rate_index];
      }
	
      for(s32 sfb=0;sfb<sfb_max;sfb++) {
	aac_unwrap(aac_bits_next_u8(b, 1, &ics->prediction_used[sfb]));
      }
    }
    
    ics->num_window_group = 1;
    ics->window_group_len[0] = 1;
  }
 
  return AAC_ERROR_NONE;
}

#endif // AAC_IMPLEMENTATION

#undef u8
#undef s8
#undef u16
#undef s16
#undef u32
#undef s32
#undef u64
#undef s64

#endif // AAC_H
