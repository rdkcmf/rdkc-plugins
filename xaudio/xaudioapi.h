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
/*
https://ccp.sys.comcast.net/browse/RDKC-3938
*/


#ifndef XAUDIOAPI_H
#define XAUDIOAPI_H

#include <stdint.h>
#include <string.h>

typedef enum
{
  rdkc_xaOpenMode_Playback,
  rdkc_xaOpenMode_Capture //NOT IMPL
} rdkc_xaOpenMode;

typedef enum
{
  rdkc_xaLogLevel_Debug,
  rdkc_xaLogLevel_Info,
  rdkc_xaLogLevel_Warn,
  rdkc_xaLogLevel_Error,
  rdkc_xaLogLevel_Fatal
} rdkc_xaLogLevel;

typedef void (*rdkc_xaLogFunc)(rdkc_xaLogLevel level, char const* fmt, ...);

typedef enum
{
  rdkx_xaAudoCodec_AAC,
  rdkx_xaAudoCodec_G711,
} rdkx_xaAudioCodec;

typedef enum 
{
  rdkc_xaAACFormat_RAW,
  rdkc_xaAACFormat_ADIF,
  rdkc_xaAACFormat_ADTS,
  rdkc_xaAACFormat_LOAS
} rdkc_xaAACFormat;

typedef enum 
{
  rdkc_xaG711Format_ALAW,
  rdkc_xaG711Format_ULAW
} rdkc_xaG711Format;

struct rdkc_xaudio_;

typedef struct rdkc_xaudio_ rdkc_xaudio;

#define DUMPNAME_MAX 128

typedef struct
{
  uint32_t codec;

  uint32_t format;

  uint32_t sample_rate;

  uint32_t channel_count;

  /*
    the dump file can be played on xcam2 with: aplay -r 44000 -c 2 -f S16_LE /opt/output.pcm
    or on desktop with: vlc --demux=rawaud --rawaud-channels 2 --rawaud-samplerate 44000 /tftpboot/output.pcm
  */
  char dump_file[DUMPNAME_MAX];

} rdkc_xaConfig;

#ifdef __cplusplus
extern "C" {
#endif

rdkc_xaudio* xaudio_open(rdkc_xaConfig* config, rdkc_xaOpenMode mode);

size_t xaudio_playback(uint8_t* buffer, size_t size, rdkc_xaudio* xa);

size_t xaudio_capture(uint8_t** buffer, /*uint32_t miliseconds, */rdkc_xaudio* xa);//captures 64 miliseconds of audio from mic 

void xaudio_close(rdkc_xaudio* xa);

void xaudio_enable_debug(bool debug);

#ifdef __cplusplus
}
#endif

#endif

