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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include "xaudioapi.h"
#include "logger.h"

long g_pid = 0;

void print_usage()
{
  LOG_INFO("Usage: xaudiotest [OPTIONS]... [FILE]\n");
  LOG_INFO("-P playback run in playback mode");
  LOG_INFO("-C capture  run in capture mode");
  LOG_INFO("-f format   file format of input file");
  LOG_INFO("            recognized formats: AAC, ULAW");
  LOG_INFO("-r rate     sample rate of input file (e.g. 11025, 44000, 48000,...)");
  LOG_INFO("-c channels channel count of input file.  must be 1 or 2");
  LOG_INFO("-d dump     path of file to dump raw decoded pcm data to");
  LOG_INFO("            the dump file can be played on xcam2 like this:")
  LOG_INFO("              aplay -r 44000 -c 2 -f S16_LE /opt/output.pcm");
  LOG_INFO("            and on a desktop like this:");
  LOG_INFO("              vlc --demux=rawaud --rawaud-channels 2 --rawaud-samplerate 44000 /tftpboot/output.pcm");
  LOG_INFO("-v verbose  enable debug logging");
  LOG_INFO("-h help     print usage");
  LOG_INFO("Examples:");
  LOG_INFO("./xaudiotest -P -f AAC -r 48000 -c 2 audio_classical_48000_2_aac");
  LOG_INFO("./xaudiotest -P -f ULAW -r 48000 -c 2 audio_classical_48000_2_ulaw");
  LOG_INFO("./xaudiotest -P -f AAC -r 11025 -c 1 audio_hiphop_11025_1_aac");
  LOG_INFO("./xaudiotest -P -f ULAW -r 11025 -c 1 audio_hiphop_11025_1_ulaw");
  LOG_INFO("./xaudiotest -C -f ULAW -r 16000 -c 1 /opt/dump.ulaw");
}

int main(int argc, char *argv[])
{
  const size_t BUFSZ = 8192;
  uint8_t buffer[BUFSZ+1];
  uint8_t* pcapture_buffer = NULL;
	int readsz;
  FILE* file;
  rdkc_xaOpenMode mode = rdkc_xaOpenMode_Playback;
  rdkc_xaudio* pxa;
  rdkc_xaConfig config;
  int option_index;
  static struct option long_options[] = 
  {
    {"format", required_argument, 0, 'f'},
    {"rate", required_argument, 0, 'r'},
    {"channels", required_argument, 0, 'c'},
    {"dump", required_argument, 0, 'd'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  memset(&config, 0, sizeof(rdkc_xaConfig));
  config.codec = rdkx_xaAudoCodec_AAC;
  config.format = rdkc_xaAACFormat_ADTS;
  config.sample_rate = 11025;
  config.channel_count = 1;

  rdkc_logger_init("LOG.RDK.XAUDIO");

  while (true)
  {
    int c = getopt_long(argc, argv, "PCr:c:f:p:d:vh", long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'P':
      mode = rdkc_xaOpenMode_Playback;
      break;

      case 'C':
      mode = rdkc_xaOpenMode_Capture;
      break;

      case 'f':
      if(strcmp("AAC", optarg) == 0)
      {
        config.codec = rdkx_xaAudoCodec_AAC;
        config.format = rdkc_xaAACFormat_ADTS;
      }
      else
      if(strcmp("ULAW", optarg) == 0)
      {
        config.codec = rdkx_xaAudoCodec_G711;
        config.format = rdkc_xaG711Format_ULAW;
      }
      else
      {
        LOG_ERROR("Invalid format: %s", optarg);
        exit(0);
      }
      break;

      case 'r':
      config.sample_rate = strtol(optarg, nullptr, 10);
      break;

      case 'c':
      config.channel_count = strtol(optarg, nullptr, 10);
      if(config.channel_count < 1 || config.channel_count > 2)
      {
        LOG_ERROR("Invalid channels: %d", config.channel_count);
        exit(0);
      }
      break;

      case 'd':
      if(strlen(optarg) >= sizeof(config.dump_file))
      {
        LOG_ERROR("dump file path too long: exceedes %d", sizeof(config.dump_file));
        exit(0);
      }
      strncpy(config.dump_file, optarg, sizeof(config.dump_file));
      break;

      case 'v':
      rdkc_logger_init("LOG.RDK.XAUDIO", false, RDK_LOG_DEBUG, true);
      xaudio_enable_debug(true);
      break;

      default:
      print_usage();
      exit(0);
      break;
    }
  }

  if(optind >= argc)
  {
    LOG_ERROR("Missing file argument");
    print_usage();
    exit(0);
  }

  file = fopen(argv[optind], mode == rdkc_xaOpenMode_Playback ? "rb" : "wb");

  if(!file)
  {
    LOG_ERROR("Failed to open input file %s", argv[optind]);
    exit(0);
  }

  if(argc > 2)
  {
    strncpy(config.dump_file, argv[2], sizeof(config.dump_file));
  }

  pxa = xaudio_open(&config, mode);

  if(!pxa)
  {
    LOG_ERROR("Failed to open pxaudio\n");
    fclose(file);
    return -1;
  }

  if(mode == rdkc_xaOpenMode_Playback)
  {
    do
    {
	    readsz = fread(buffer, 1, BUFSZ, file);

	    if(readsz > 0)
      {
        xaudio_playback(buffer, readsz, pxa);
      }

    } while(readsz == BUFSZ);
  }
  else
  {
    do
    {
      readsz = xaudio_capture(&pcapture_buffer, pxa);

      if(readsz > 0)
      {
        fwrite(pcapture_buffer, sizeof(uint8_t), readsz, file);
      }
    } while(1);
  }

  xaudio_close(pxa);
  fclose(file);
}



