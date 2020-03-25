/*
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
*/
#include "xaudioapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <thread>
#include <alsa/asoundlib.h>
#include "aac_audio_dec.h"
#include "aac_audio_enc.h"
#include "logger.h"
#include "g711.h"

#define USE_ALSA_G711_DECODE 0

class AudioCodec;
class AudioDevice;

bool g_debug_enabled = false;

struct rdkc_xaudio_
{
  rdkc_xaOpenMode mode;

  rdkc_xaConfig config;

  AudioCodec* codec;

  AudioDevice* device;

  int16_t* pcm_buffer;
  size_t pcm_buffer_size;

  std::thread* capture_thread;

  FILE* dump_file;
};

const char* getAmbaDecStatusMessage(s32 code)
{
  switch(code)
  {
    case AAC_DECODE_OK: return "OK";
    case AAC_DECODE_UNSUPPORTED_FORMAT: return "AAC_DECODE_UNSUPPORTED_FORMAT";
    case AAC_DECODE_DECODE_FRAME_ERROR: return "AAC_DECODE_DECODE_FRAME_ERROR";
    case AAC_DECODE_CRC_CHECK_ERROR: return "AAC_DECODE_CRC_CHECK_ERROR";
    case AAC_DECODE_INVALID_CODE_BOOK: return "AAC_DECODE_INVALID_CODE_BOOK";
    case AAC_DECODE_UNSUPPORTED_WINOW_SHAPE: return "AAC_DECODE_UNSUPPORTED_WINOW_SHAPE";
    case AAC_DECODE_PREDICTION_NOT_SUPPORTED_IN_LC_AAC: return "AAC_DECODE_PREDICTION_NOT_SUPPORTED_IN_LC_AAC";
    case AAC_DECODE_UNIMPLEMENTED_CCE: return "AAC_DECODE_UNIMPLEMENTED_CCE";
    case AAC_DECODE_UNIMPLEMENTED_GAIN_CONTROL_DATA: return "AAC_DECODE_UNIMPLEMENTED_GAIN_CONTROL_DATA";
    case AAC_DECODE_UNIMPLEMENTED_EP_SPECIFIC_CONFIG_PARSE: return "AAC_DECODE_UNIMPLEMENTED_EP_SPECIFIC_CONFIG_PARSE";
    case AAC_DECODE_UNIMPLEMENTED_CELP_SPECIFIC_CONFIG_PARSE: return "AAC_DECODE_UNIMPLEMENTED_CELP_SPECIFIC_CONFIG_PARSE";
    case AAC_DECODE_UNIMPLEMENTED_HVXC_SPECIFIC_CONFIG_PARSE: return "AAC_DECODE_UNIMPLEMENTED_HVXC_SPECIFIC_CONFIG_PARSE";
    case AAC_DECODE_OVERWRITE_BITS_IN_INPUT_BUFFER: return "AAC_DECODE_OVERWRITE_BITS_IN_INPUT_BUFFER";
    case AAC_DECODE_CANNOT_REACH_BUFFER_FULLNESS: return "AAC_DECODE_CANNOT_REACH_BUFFER_FULLNESS";
    case AAC_DECODE_TNS_RANGE_ERROR: return "AAC_DECODE_TNS_RANGE_ERROR";
    default: return "UNKNOWN";
  };
  return "UNKNOWN";
}

const char* getAmbaEncStatusMessage(s32 code)
{
  switch(code)
  {
    case AAC_ENCODE_OK: return "ENCODE_OK";
    case AAC_ENCODE_INVALID_POINTER: return "ENCODE_INVALID_POINTER";
    case AAC_ENCODE_FAILED: return "ENCODE_FAILED";
    case AAC_ENCODE_UNSUPPORTED_SAMPLE_RATE: return "ENCODE_UNSUPPORTED_SAMPLE_RATE";
    case AAC_ENCODE_UNSUPPORTED_CH_CFG: return "ENCODE_UNSUPPORTED_CH_CFG";
    case AAC_ENCODE_UNSUPPORTED_BIT_RATE: return "ENCODE_UNSUPPORTED_BIT_RATE";
    case AAC_ENCODE_UNSUPPORTED_MODE: return "ENCODE_UNSUPPORTED_MODE";
    default: return "UNKNOWN";
  };
  return "UNKNOWN";
}

//covert from 32 bit pcm to 16 bit pcm
static void fc32ito16i(s32 *bufin, s16 *bufout, s32 ch, s32 proc_size)
{
	s32 i, j;
	s32 *bufin_ptr;
	s16 *bufout_ptr;

	bufin_ptr = bufin;
	bufout_ptr = bufout;
	for (i = 0; i < proc_size; i++) {
		for(j = 0; j < ch; j++) {
			*bufout_ptr = (*bufin_ptr) >> 16;
			bufin_ptr++;
			bufout_ptr++;
		}
	}
}

class AudioCodec
{
public:
  AudioCodec(rdkc_xaudio_* p) : pxa(p)
  {
  }

  virtual ~AudioCodec()
  {
  }

  virtual void open() = 0;

  virtual void close() = 0;

  virtual size_t decode(uint8_t* input_buffer, size_t input_size, 
                        int16_t* output_buffer, size_t output_size) = 0;

  virtual size_t encode(int16_t* input_buffer, size_t input_size, 
                        uint8_t* output_buffer, size_t output_size) = 0;

protected:

  rdkc_xaudio_* pxa;

};

class AACCodec : public AudioCodec
{
public:
  AACCodec(rdkc_xaudio_* p) : AudioCodec(p)
  {
  }

  ~AACCodec()
  {
    close();
  }

  void open()
  {
    if(pxa->mode == rdkc_xaOpenMode_Playback)
    {
      memset(&au_confdec, 0, sizeof(au_aacdec_config_t));

	    au_confdec.bsFormat              = pxa->config.format;	
	    au_confdec.outNumCh              = pxa->config.channel_count;
	    au_confdec.codec_lib_mem_size    = aacdec_get_mem_size(6);
	    au_confdec.codec_lib_mem_addr    = (u32*)malloc(au_confdec.codec_lib_mem_size);

	    aacdec_setup(&au_confdec);
      if(au_confdec.ErrorStatus)
      {
        LOG_ERROR("aacdec_setup failed error=(%d) %s", au_confdec.ErrorStatus, getAmbaDecStatusMessage(au_confdec.ErrorStatus));
      }

	    aacdec_open(&au_confdec);
      if(au_confdec.ErrorStatus)
      {
        LOG_ERROR("aacdec_open failed error=(%d) %s", au_confdec.ErrorStatus, getAmbaDecStatusMessage(au_confdec.ErrorStatus));
      }
    }
    else
    {
      memset(&au_confenc, 0, sizeof(au_aacenc_config_t));
	    au_confenc.enc_mode		      = 0;// 0: AAC; 1: AAC_PLUS; 3: AACPLUS_PS;
	    au_confenc.sample_freq		  = 16000;//pxa->config.sample_rate;
	    au_confenc.coreSampleRate	  = 48000;
	    au_confenc.Src_numCh		    = 1;//this is the source/microphone which on the cams is mono
	    au_confenc.tns				      = 1;
      if(pxa->config.format == rdkc_xaAACFormat_ADTS)
        au_confenc.ffType			    = 't';
      else if(pxa->config.format == rdkc_xaAACFormat_ADIF)
        au_confenc.ffType			    = 'a';
      else if(pxa->config.format == rdkc_xaAACFormat_LOAS)
        au_confenc.ffType			    = 'l';
      else if(pxa->config.format == rdkc_xaAACFormat_RAW)
        au_confenc.ffType			    = 'r';
      else
        au_confenc.ffType			    = 't';
	    au_confenc.bitRate			    = 64000;//128000; // AAC: 128000; AAC_PLUS: 64000; AACPLUS_PS: 40000;
	    au_confenc.quantizerQuality	= 2; //0=low, 1=high, 2=highest
	    au_confenc.codec_lib_mem_size = aacenc_get_mem_size(au_confenc.Src_numCh, au_confenc.sample_freq, au_confenc.bitRate, au_confenc.enc_mode);
	    au_confenc.codec_lib_mem_adr= (u32 *)malloc(au_confenc.codec_lib_mem_size);

    	LOG_INFO("aac encoder settings: \tmode(%d), sfreq(%d), coresfreq(%d), bitrate(%d), srcch(%d), quality(%d), bitrate(%d)\n",
		   au_confenc.enc_mode, 
		   au_confenc.sample_freq,
		   au_confenc.coreSampleRate,
		   au_confenc.bitRate,
		   au_confenc.Src_numCh,
		   au_confenc.quantizerQuality,
       au_confenc.bitRate);

    	aacenc_setup(&au_confenc);
      if(au_confenc.ErrorStatus != AAC_ENCODE_OK)
      {
        LOG_ERROR("aacenc_setup failed error=(%d) %s", au_confenc.ErrorStatus, getAmbaEncStatusMessage(au_confenc.ErrorStatus));
      }

    	aacenc_open(&au_confenc);
      if(au_confenc.ErrorStatus != AAC_ENCODE_OK)
      {
        LOG_ERROR("aacenc_open failed error=(%d) %s", au_confenc.ErrorStatus, getAmbaEncStatusMessage(au_confenc.ErrorStatus));
      }
    }
  }

  void close()
  {
    //aacdec_close(); //TODO this is in header file but not in sample code -- is this needed ???
    if (au_confenc.codec_lib_mem_adr)
      free(au_confenc.codec_lib_mem_adr);
    if (au_confdec.codec_lib_mem_addr)
      free(au_confdec.codec_lib_mem_addr);
  }

  size_t decode(uint8_t* input_buffer, size_t input_size, int16_t* output_buffer, size_t output_size)
  {
    int output_index = 0;
    //s32 output_buf[sizeof(s32)*2*SAMPLES_PER_FRAME];
    s32 output_buf[sizeof(s32)*2*4096];
    u32 interBufSize = 0;

	  au_confdec.dec_rptr = input_buf;
	  au_confdec.dec_wptr = output_buf;

    if(INPUT_BUF_SIZE - interBufSize > input_size)
    {
      memcpy(au_confdec.dec_rptr + interBufSize, input_buffer, input_size);
      interBufSize += input_size;
    }
    else
    {
      LOG_WARN("Unexpected not enough space");
      return 0;
    }

    do
    {
	    aacdec_decode(&au_confdec);

      if(au_confdec.consumedByte > 0 && !au_confdec.ErrorStatus)
      {
        au_confdec.dec_rptr = input_buf;

        if (interBufSize >= au_confdec.consumedByte)
        {
          interBufSize -= au_confdec.consumedByte;
        }
        else
        {
          interBufSize = 0;
        }

		    memmove(au_confdec.dec_rptr, 
                au_confdec.dec_rptr + au_confdec.consumedByte, 
                interBufSize);

		    au_confdec.consumedByte = 0;

        fc32ito16i(au_confdec.dec_wptr,
                   &output_buffer[output_index],
                   au_confdec.outNumCh,
                   au_confdec.frameSize);
        //Note that when tested:
        // SAMPLES_PER_FRAME = 4096
        //  au_confdec.frameSize = 1024
        output_index += au_confdec.outNumCh * au_confdec.frameSize;

        if(output_index >= (int)output_size && !au_confdec.ErrorStatus)
        {
          LOG_WARN("output_buffer too small");
        }
      }

    } while(!au_confdec.ErrorStatus && output_index < (int)output_size);

    return output_index / au_confdec.outNumCh;
  }

  size_t encode(int16_t* input_buffer, size_t input_size, 
                uint8_t* output_buffer, size_t output_size)
  {
    (void)output_size;
    //i need to take batch_size number of frames and since a frame is 2 bytes(int16_t), this will be batch_size*2 bytes
    size_t batch_size = 1024 * au_confenc.sample_freq * au_confenc.Src_numCh/au_confenc.coreSampleRate;
    size_t encoded_bytes = 0;
    LOG_WARN("encode input_size=%lu, batch_size=%lu nBitsInRawDataBlock=%d", input_size, batch_size, au_confenc.nBitsInRawDataBlock);
    for(size_t batch = 0; batch < input_size / batch_size; ++batch)//TODO -- we pray that input_size is equally divisible by batch_size
    {
//    LOG_WARN("%d: au_confenc.nBitsInRawDataBlock=%d", (int)batch, au_confenc.nBitsInRawDataBlock);
		  au_confenc.enc_rptr = (s32*)(input_buffer + batch * batch_size);
		  au_confenc.enc_wptr = output_buffer + encoded_bytes;
      aacenc_encode(&au_confenc);
      encoded_bytes += ((au_confenc.nBitsInRawDataBlock + 7) >> 3);
    }
    LOG_WARN("encoded_bytes = %lu", encoded_bytes);
    return encoded_bytes;
  }
private:

  static const int SHARED_BUF_SIZE = 25000;
  static const int INPUT_BUF_SIZE = 16384;

  u32 shared_buf[SHARED_BUF_SIZE];
  u8 input_buf[INPUT_BUF_SIZE];

  au_aacdec_config_t au_confdec;
  au_aacenc_config_t au_confenc;
};


class G711Codec : public AudioCodec
{
public:
  G711Codec(rdkc_xaudio_* p) : AudioCodec(p)
  {
  }

  ~G711Codec()
  {
    close();
  }

  void open()
  {
  }

  void close()
  {
  }

  size_t decode(uint8_t* input_buffer, size_t input_size, 
                int16_t* output_buffer, size_t output_size)
  {
    if(input_size > output_size)
    { 
      LOG_ERROR("g711 input buffer too small");
      exit(0);
    }

    for(size_t i = 0; i < input_size; ++i)
    {         
      output_buffer[i] = ulaw_to_linear(input_buffer[i]);
    }

    return input_size / pxa->config.channel_count;
  }

  size_t encode(int16_t* input_buffer, size_t input_size, 
                uint8_t* output_buffer, size_t output_size)
  {
    if(input_size > output_size)
    { 
      LOG_ERROR("g711 input buffer too small");
      exit(0);
    }

    for(size_t i = 0; i < input_size; ++i)
    {         
      output_buffer[i] = linear_to_ulaw(input_buffer[i]);
    }

    return input_size / pxa->config.channel_count;
  }
};


class AudioDevice
{
public:
  AudioDevice(rdkc_xaudio_* p) : pxa(p)
  {
  }

  virtual ~AudioDevice()
  {
  }

  virtual void open() = 0;

  virtual void close() = 0;

  virtual void write(uint8_t* pcm_buffer, size_t num_frames) = 0;

  virtual void read(uint8_t* pcm_buffer, size_t num_frames) = 0;

protected:

  rdkc_xaudio_* pxa;

};

#define ALSA_CHK_1(FUNC) \
  if ((err = FUNC) < 0) { \
    LOG_ERROR("%s: %s", #FUNC, snd_strerror(err)); \
    exit(1); \
  } else { \
    LOG_INFO("%s: ok", #FUNC); \
  }

class AlsaDevice : public AudioDevice
{
public:
  AlsaDevice(rdkc_xaudio_* p) : AudioDevice(p), pcm(NULL)
  {
  }

  ~AlsaDevice()
  {
    close();
  }

  void open()
  {
    int err;
    snd_pcm_hw_params_t* params;

    if(pxa->mode == rdkc_xaOpenMode_Playback)
    {
      channel_count = pxa->config.channel_count;
      sample_rate = pxa->config.sample_rate;
    }
    else
    {
      channel_count = 1;
      sample_rate = 22050;//16000;
    }

    period_size = 1024;//1024 working good so far

    //TODO support SND_PCM_NONBLOCK | SND_PCM_ASYNC
    if(pxa->mode == rdkc_xaOpenMode_Playback)
    {
      ALSA_CHK_1( snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0) );
    }
    else
    {
      ALSA_CHK_1( snd_pcm_open(&pcm, "mixin", SND_PCM_STREAM_CAPTURE, 0) );
    }

    ALSA_CHK_1( snd_pcm_hw_params_malloc(&params) );
    ALSA_CHK_1( snd_pcm_hw_params_any(pcm, params) );
#if USE_ALSA_G711_DECODE
    if(pxa->config.codec == rdkx_xaAudoCodec_G711)
    {
      frame_size = 1;
      if(pxa->config.format == rdkc_xaG711Format_ALAW)
      {
        ALSA_CHK_1( snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_A_LAW) );
      }
      else
      if(pxa->config.format == rdkc_xaG711Format_ULAW)
      {
        ALSA_CHK_1( snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_MU_LAW) );
      }
      else
      {
        LOG_ERROR("unexpected g711 format %d", pxa->config.format);
      }
    }
    else
#endif
    {
      ALSA_CHK_1( snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_S16_LE) );
      frame_size = 2;
    }
      
    ALSA_CHK_1( snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED) );
    ALSA_CHK_1( snd_pcm_hw_params_set_period_size_near(pcm, params, &period_size, 0) );
    ALSA_CHK_1( snd_pcm_hw_params_set_rate_near(pcm, params, &sample_rate, 0) );
    ALSA_CHK_1( snd_pcm_hw_params_set_channels(pcm, params, channel_count) );
    ALSA_CHK_1( snd_pcm_hw_params(pcm, params) );
    ALSA_CHK_1( snd_pcm_prepare(pcm) );
    snd_pcm_hw_params_free(params);

    LOG_INFO("alsa configured with period_size=%lu sample_rate=%u", period_size, sample_rate);
  }

  void close()
  {
    if(pcm)
    {
      //snd_pcm_drain(pcm);TODO
      snd_pcm_close(pcm);
      pcm = NULL;
    }
  }

  void write(uint8_t* pcm_buffer, size_t num_frames)
  {
    int remaining = num_frames % period_size;
    int num_batches = num_frames / period_size;
    LOG_DEBUG("write frames=%d period=%lu batches=%d remaining=%d channels=%u framesz=%u", num_frames, period_size, num_batches, remaining, channel_count, frame_size);

    for(int batch = 0; batch < num_batches; ++batch)
    {
      int offset = (batch * period_size) * channel_count * frame_size;
      int frames = period_size;
      snd_pcm_sframes_t  ret = snd_pcm_writei(pcm, pcm_buffer + offset, frames);
      if(ret < 0)
      {
        LOG_WARN("snd_pcm_writei error %lu so recovering", ret);
        snd_pcm_recover(pcm, ret, 0);
      }
      else if (ret != static_cast<snd_pcm_sframes_t>(frames))
      {
        LOG_ERROR("snd_pcm_writei frame count %lu does not match %d", ret, frames);
      }
    }
  }

  void read(uint8_t* pcm_buffer, size_t num_frames)
  {
    int remaining = num_frames % period_size;
    int num_batches = num_frames / period_size;
    LOG_DEBUG("read frames=%d period=%lu batches=%d remaining=%d channels=%u framesz=%u", num_frames, period_size, num_batches, remaining, channel_count, frame_size);

    for(int batch = 0; batch < num_batches; ++batch)
    {
      int offset = (batch * period_size) * channel_count * frame_size;
      int frames = period_size;

      snd_pcm_sframes_t ret = snd_pcm_readi(pcm, pcm_buffer + offset, frames);
      if(ret < 0)//error case
      {
          LOG_WARN("snd_pcm_readi error %lu so recovering", ret);
          snd_pcm_recover(pcm, ret, 0);
      }
      else if (ret != static_cast<snd_pcm_sframes_t>(frames))
      {
        LOG_ERROR("snd_pcm_readi frame count %lu does not match %d", ret, frames);
      }
    }
  }


private:

  snd_pcm_t* pcm;
  snd_pcm_uframes_t period_size;
  unsigned int channel_count;
  unsigned int sample_rate;
  unsigned int frame_size;
};

void capture_thread_func(rdkc_xaudio_* pxa)
{
  (void)pxa;
}

rdkc_xaudio_* xaudio_open(rdkc_xaConfig* config, rdkc_xaOpenMode mode)
{
  if(g_debug_enabled)
  {
    rdkc_logger_init("LOG.RDK.XAUDIO", false, RDK_LOG_DEBUG, true);
  }
  else
  {
    rdkc_logger_init("LOG.RDK.XAUDIO");
  }

  rdkc_xaudio_* pxa = new rdkc_xaudio_;
  
  memset(pxa, 0, sizeof(rdkc_xaudio_));

  memcpy(&pxa->config, config, sizeof(rdkc_xaConfig));

  pxa->mode = mode;

  if(pxa->config.codec == rdkx_xaAudoCodec_AAC)
  {
    pxa->codec = new AACCodec(pxa);
  }
#if !USE_ALSA_G711_DECODE
  else if(pxa->config.codec == rdkx_xaAudoCodec_G711)
  {
    pxa->codec = new G711Codec(pxa);
  }
#endif
  else
  {
    LOG_ERROR("No codec set in config");
    delete pxa;
    return NULL;
  }

  pxa->device = new AlsaDevice(pxa);

#if USE_ALSA_G711_DECODE
  if(pxa->config.codec != rdkx_xaAudoCodec_G711)
#endif
  pxa->codec->open();

  pxa->device->open();

  pxa->pcm_buffer = new int16_t[pxa->pcm_buffer_size = 1024*128];//TODO better estimate what we really need

  if(mode == rdkc_xaOpenMode_Capture)
  {
    //pxa->capture_thread = new std::thread(capture_thread_func, pxa);
  }

  if(pxa->config.dump_file[0])
  {
    pxa->dump_file = fopen(pxa->config.dump_file, "w+b");

    if(!pxa->dump_file)
    {
      LOG_ERROR("Failed to open dump file %s", pxa->config.dump_file);
    }
  }

  return pxa;
}

size_t xaudio_playback(uint8_t* buffer, size_t size, rdkc_xaudio* xa)
{
  rdkc_xaudio_* pxa = (rdkc_xaudio_*)xa;

#if USE_ALSA_G711_DECODE
  if(pxa->config.codec == rdkx_xaAudoCodec_G711)
  {
    pxa->device->write(buffer, size / pxa->config.channel_count);
    return size;
  }
  else
  {
#endif

  size_t num_frames = pxa->codec->decode(buffer, size, pxa->pcm_buffer, pxa->pcm_buffer_size);

  pxa->device->write((uint8_t*)pxa->pcm_buffer, num_frames);

  if(pxa->dump_file)
  {
    fwrite(pxa->pcm_buffer, sizeof(int16_t) * pxa->config.channel_count, num_frames, pxa->dump_file);
  }

  return num_frames;

#if USE_ALSA_G711_DECODE
  }
#endif  
}

size_t xaudio_capture(uint8_t** buffer, /*uint32_t miliseconds, */rdkc_xaudio* xa)
{
  rdkc_xaudio_* pxa = (rdkc_xaudio_*)xa;

  size_t num_frames = 1024;

  pxa->device->read((uint8_t*)pxa->pcm_buffer, num_frames);

  static uint8_t* output_buffer = new uint8_t[1024*16];
  *buffer = output_buffer;
  

  size_t encoded_bytes = pxa->codec->encode(pxa->pcm_buffer, num_frames, output_buffer, 1024*16);

  if(pxa->dump_file)
  {
    fwrite(output_buffer, sizeof(int8_t), encoded_bytes, pxa->dump_file);
  }
  return encoded_bytes;
}

void xaudio_close(rdkc_xaudio* xa)
{
  rdkc_xaudio_* pxa = (rdkc_xaudio_*)xa;

  if(pxa->pcm_buffer)
  {
    delete [] pxa->pcm_buffer;
    pxa->pcm_buffer = NULL;
  }

  if(pxa->codec)
  {
    delete pxa->codec;
    pxa->codec = NULL;
  }

  if(pxa->device)
  {
    delete pxa->device;
    pxa->device = NULL;
  }

  if(pxa->dump_file)
  {
    fclose(pxa->dump_file);
    pxa->dump_file = NULL;
  }
}

void xaudio_enable_debug(bool debug)
{
  g_debug_enabled = debug;
  rdkc_logger_init("LOG.RDK.XAUDIO", false, RDK_LOG_DEBUG, true);
}

