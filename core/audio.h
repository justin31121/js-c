#ifndef AUDIO_H
#define AUDIO_H

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

// win32
//   mingw: -lole32 -lxaudio2_8
//   msvc : ole32.lib

// linux
//   gcc  : -lasound

#ifdef _WIN32
#  include <xaudio2.h>
#elif linux
#  include <alsa/asoundlib.h>
#endif //_WIN32

#ifndef AUDIO_DEF
#  define AUDIO_DEF static inline
#endif //AUDIO_DEF

typedef enum{
  AUDIO_FMT_S16,
  AUDIO_FMT_FLT,
}Audio_Fmt;

typedef struct{
  int channels;
  int sample_rate;
  int sample_size;

#ifdef _WIN32
  IXAudio2SourceVoice *xaudio2_source_voice;
  HANDLE semaphore;
#elif linux
  snd_pcm_t *alsa_snd_pcm;
#endif
}Audio;

AUDIO_DEF int audio_init(Audio *audio, Audio_Fmt fmt, int channels, int sample_rate);
AUDIO_DEF void audio_play(Audio *audio, unsigned char *data, int samples);
//AUDIO_DEF void audio_play_async(Audio *audio, unsigned char *data, int samples);
AUDIO_DEF void audio_block(Audio *audio);
AUDIO_DEF void audio_free(Audio *audio);

AUDIO_DEF int audio_fmt_bits_per_sample(int *bits, Audio_Fmt fmt);

#ifdef AUDIO_IMPLEMENTATION

#ifdef _WIN32

static int audio_co_initialized = 0;
static IXAudio2* audio_xaudio2 = NULL;
static IXAudio2MasteringVoice *audio_xaudio2_mastering_voice = NULL;

AUDIO_DEF void audio_xaudio2_OnBufferEnd(IXAudio2VoiceCallback* This, void* pBufferContext);
AUDIO_DEF void audio_xaudio2_OnStreamEnd(IXAudio2VoiceCallback* This);
AUDIO_DEF void audio_xaudio2_OnVoiceProcessingPassEnd(IXAudio2VoiceCallback* This);
AUDIO_DEF void audio_xaudio2_OnVoiceProcessingPassStart(IXAudio2VoiceCallback* This, UINT32 SamplesRequired);
AUDIO_DEF void audio_xaudio2_OnBufferStart(IXAudio2VoiceCallback* This, void* pBufferContext);
AUDIO_DEF void audio_xaudio2_OnLoopEnd(IXAudio2VoiceCallback* This, void* pBufferContext);
AUDIO_DEF void audio_xaudio2_OnVoiceError(IXAudio2VoiceCallback* This, void* pBufferContext, HRESULT Error);

static IXAudio2VoiceCallback audio_xaudio2_callbacks = {
  .lpVtbl = &(IXAudio2VoiceCallbackVtbl) {
    .OnStreamEnd = audio_xaudio2_OnStreamEnd,
    .OnVoiceProcessingPassEnd = audio_xaudio2_OnVoiceProcessingPassEnd,
    .OnVoiceProcessingPassStart = audio_xaudio2_OnVoiceProcessingPassStart,
    .OnBufferEnd = audio_xaudio2_OnBufferEnd,
    .OnBufferStart = audio_xaudio2_OnBufferStart,
    .OnLoopEnd = audio_xaudio2_OnLoopEnd,
    .OnVoiceError = audio_xaudio2_OnVoiceError,
  }
};

AUDIO_DEF int audio_fmt_format_tag(int *tag, Audio_Fmt fmt) {
  switch(fmt) {
  case AUDIO_FMT_S16: {
    *tag = WAVE_FORMAT_PCM;
    return 1;
  } break;
  case AUDIO_FMT_FLT: {
    *tag = WAVE_FORMAT_IEEE_FLOAT;
    return 1;
  } break;
  default: {
    return 0;	
  } break;
  }
}

AUDIO_DEF int audio_init(Audio *audio, Audio_Fmt fmt, int channels, int sample_rate) {    
  audio->sample_rate = sample_rate;
  audio->channels    = channels;

  int bits_per_sample;
  if(!audio_fmt_bits_per_sample(&bits_per_sample, fmt)) {
    return 0;
  }
  int format_tag;
  if(!audio_fmt_format_tag(&format_tag, fmt)) {
    return 0;
  }
  audio->sample_size = bits_per_sample * channels / 8;

  if( !audio_co_initialized ) {
    if( FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) ) {
      return 0;
    }
    audio_co_initialized = 1;
  } 

  if( !audio_xaudio2 &&
      FAILED(XAudio2Create(&audio_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)) ) {
    return 0;
  }

  if(!audio_xaudio2_mastering_voice &&
     FAILED(audio_xaudio2->lpVtbl->
	    CreateMasteringVoice(audio_xaudio2,
				 &audio_xaudio2_mastering_voice,
				 channels,
				 sample_rate,
				 0,
				 0,
				 NULL,
				 AudioCategory_GameEffects)) ) {
    return 0;
  }

  WAVEFORMATEX wave_format;
  wave_format.wFormatTag = (WORD) format_tag;
  wave_format.nChannels = (WORD) channels;
  wave_format.nSamplesPerSec = sample_rate;
  wave_format.wBitsPerSample = (WORD) bits_per_sample;
  wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
  wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
  wave_format.cbSize = 0;
  audio->xaudio2_source_voice = NULL;
  if( FAILED(audio_xaudio2->lpVtbl->
	     CreateSourceVoice(audio_xaudio2,
			       &audio->xaudio2_source_voice,
			       &wave_format,
			       0,
			       1.f,
			       &audio_xaudio2_callbacks,
			       NULL,
			       NULL)) ) {
    return 0;
  }

  audio->xaudio2_source_voice->lpVtbl->Start(audio->xaudio2_source_voice, 0, XAUDIO2_COMMIT_NOW);    
  audio->semaphore = CreateSemaphore(NULL, 0, 1, NULL);
  ReleaseSemaphore(audio->semaphore, 1, NULL);
    
  if(GetLastError() == ERROR_ALREADY_EXISTS) {
    return 0;
  }
    
  return 1;
}

AUDIO_DEF void audio_play(Audio *audio, unsigned char *data, int samples) {
  XAUDIO2_BUFFER xaudioBuffer = {0};
  
  xaudioBuffer.AudioBytes = samples * audio->sample_size;
  xaudioBuffer.pAudioData = data;
  xaudioBuffer.pContext = audio;
    
  audio->xaudio2_source_voice->lpVtbl->SubmitSourceBuffer(audio->xaudio2_source_voice, &xaudioBuffer, NULL);

  WaitForSingleObject((audio)->semaphore, INFINITE);
}

AUDIO_DEF void audio_play_async(Audio *audio, unsigned char *data, int samples) {
  XAUDIO2_BUFFER xaudioBuffer = {0};
  
  xaudioBuffer.AudioBytes = samples * audio->sample_size;
  xaudioBuffer.pAudioData = data;
  xaudioBuffer.pContext = audio;
    
  audio->xaudio2_source_voice->lpVtbl->SubmitSourceBuffer(audio->xaudio2_source_voice, &xaudioBuffer, NULL);
}

AUDIO_DEF void audio_block(Audio *audio) {
  while(1) {
    Sleep(10);
    DWORD result = WaitForSingleObject(audio->semaphore, 0);
    if(result == WAIT_OBJECT_0) {
      return;
    }
  }
}

AUDIO_DEF void audio_free(Audio *audio) {
  audio->xaudio2_source_voice->lpVtbl->Stop(audio->xaudio2_source_voice, 0, 0);
  audio->xaudio2_source_voice->lpVtbl->DestroyVoice(audio->xaudio2_source_voice);
  CloseHandle(audio->semaphore);
}

AUDIO_DEF void audio_xaudio2_OnBufferEnd(IXAudio2VoiceCallback* This, void* pBufferContext) {
  (void) This;
  Audio *audio = (Audio *) pBufferContext;
  ReleaseSemaphore(audio->semaphore, 1, NULL);
}
AUDIO_DEF void audio_xaudio2_OnBufferStart(IXAudio2VoiceCallback* This, void* pBufferContext) { (void) This; (void) pBufferContext; }
AUDIO_DEF void audio_xaudio2_OnStreamEnd(IXAudio2VoiceCallback* This) { (void) This; }
AUDIO_DEF void audio_xaudio2_OnVoiceProcessingPassEnd(IXAudio2VoiceCallback* This) { (void) This; }
AUDIO_DEF void audio_xaudio2_OnVoiceProcessingPassStart(IXAudio2VoiceCallback* This, UINT32 SamplesRequired) { (void) This; (void) SamplesRequired; }
AUDIO_DEF void audio_xaudio2_OnLoopEnd(IXAudio2VoiceCallback* This, void* pBufferContext) { (void) This; (void) pBufferContext; }
AUDIO_DEF void audio_xaudio2_OnVoiceError(IXAudio2VoiceCallback* This, void* pBufferContext, HRESULT Error) { (void) This; (void) pBufferContext, (void) Error; }

#elif linux

AUDIO_DEF int audio_fmt_pcm_format(snd_pcm_format_t *format, Audio_Fmt fmt) {
  switch(fmt) {
  case AUDIO_FMT_S16: {
    *format = SND_PCM_FORMAT_S16_LE;
    return 1;
  } break;
  case AUDIO_FMT_FLT: {
    *format = SND_PCM_FORMAT_FLOAT_LE;
    return 1;
  } break;
  default: {
    return 0;
  } break;
  }
}

AUDIO_DEF int audio_init(Audio *audio, Audio_Fmt fmt, int channels, int sample_rate) {
  if(snd_pcm_open(&audio->alsa_snd_pcm,
		  "default",
		  SND_PCM_STREAM_PLAYBACK,
		  SND_PCM_ASYNC) < 0) {
    return 0;
  }

  int bits_per_sample;
  if(!audio_fmt_bits_per_sample(&bits_per_sample, fmt)) {
    return 0;
  }
  audio->sample_size = bits_per_sample * channels / 8;

  snd_pcm_format_t snd_pcm_format;
  if(!audio_fmt_pcm_format(&snd_pcm_format, fmt)) {
    return 0;
  }
  
  if(snd_pcm_set_params(audio->alsa_snd_pcm,
		        snd_pcm_format,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			channels,
			sample_rate,
			1,
			sample_rate / 4) < 0) {
    return 0;
  }
  snd_pcm_uframes_t buffer_size = 0;
  snd_pcm_uframes_t period_size = 0;
  if(snd_pcm_get_params(audio->alsa_snd_pcm, &buffer_size, &period_size) < 0) {
    return 0;
  }
  snd_pcm_prepare(audio->alsa_snd_pcm);

  return 1;  
}

AUDIO_DEF void audio_play(Audio *audio, unsigned char *data, int samples) {

  while(samples > 0) {
    int ret = snd_pcm_writei(audio->alsa_snd_pcm, data, samples);
    if(ret <= 0) {
      snd_pcm_recover(audio->alsa_snd_pcm, ret, 1);
      //snd_pcm_prepare(audio->device);
    } else {
      samples -= ret;
    }
  }

}

AUDIO_DEF void audio_free(Audio *audio) {
  snd_pcm_drain(audio->alsa_snd_pcm);
  snd_pcm_close(audio->alsa_snd_pcm);
  audio->alsa_snd_pcm = NULL;
}

AUDIO_DEF void audio_block(Audio *audio) {

  (void) audio;
  // alsa by default blocks the execution, unlike xaudio2

}

#endif //_WIN32

AUDIO_DEF int audio_fmt_bits_per_sample(int *bits, Audio_Fmt fmt) {
  switch(fmt) {
  case AUDIO_FMT_S16: {
    *bits = 16;
    return 1;
  } break;
  case AUDIO_FMT_FLT: {
    *bits = 32;
    return 1;
  } break;
  default: {
    return 0;	
  } break;
  }
}

#endif //AUDIO_IMPLEMENTATION

#endif //AUDIO_H
